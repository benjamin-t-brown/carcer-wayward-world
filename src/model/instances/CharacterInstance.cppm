module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.instances:CharacterInstance;
export import bmin.containers;
import bmin.string_interop;
export import carcer.db;
export import carcer.model.templates;
import sdl2w;
#include "macros.h"

export {

// --- from model/instances/CharacterInstance.h ---
namespace model {

enum class CharacterFacing { Right, Left };

// Up, up-right, right, down-right → Right; left, up-left, down-left, down → Left.
inline CharacterFacing facingFromMoveDelta(int dx, int dy) {
  if (dx < 0 || (dx == 0 && dy > 0)) {
    return CharacterFacing::Left;
  }
  return CharacterFacing::Right;
}

// Map-entity character (player avatar, future NPCs). Distinct from CharacterPlayer
// (party chrome). Position is tile coords in map space, same as TileInstance.
struct CharacterInstance {
  bmin::String id;
  bmin::String name;
  bmin::String templateName;
  int x = 0;
  int y = 0;
  // Original map spawn tile; used to persist defeated map-placed characters after
  // movement.
  int spawnX = -1;
  int spawnY = -1;
  // Combat runtime (meaningful while world.combat.active).
  int currentAp = 0;
  int currentHp = 0; // enemies only; party HP lives on CharacterPlayer
  // Cached from CharacterTemplate.combat.hp (enemies/NPCs); party HP lives on
  // CharacterPlayer.
  int maxHp = 0;
  // True once currentHp has been set from combat (distinguishes 0 HP from uninitialized).
  bool hpInitialized = false;
  // enemies/NPCs only; party MP lives on CharacterPlayer.
  int currentMp = 0;
  // Cached from CharacterTemplate.combat.mp (enemies/NPCs).
  int maxMp = 0;
  // True once currentMp has been set from combat (distinguishes 0 MP from uninitialized).
  bool mpInitialized = false;
  // Transient pose offset (e.g. weapon swing frame); reset via
  // CharacterSetSpriteIndexOffset.
  int spriteIndexOffset = 0;
  // Default art faces right; left uses horizontal flip at render time.
  CharacterFacing facing = CharacterFacing::Right;
  // Map AI: set when IMMOBILE_UNTIL_ENEMY_SPOTTED spots the party (not persisted).
  bool agitated = false;

  // Cached from CharacterTemplate at spawn / active-map hoist (for AI without DB
  // lookups).
  CharacterTemplateType type = CharacterTemplateType::TOWNSPERSON;
  bmin::String label;
  bmin::String behaviorName;
  int visionRadius = 0;
  CombatBehaviorName combatBehaviorTown = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combatBehaviorCombat = CombatBehaviorName::SEEK_AND_MELEE;
  CharacterStats stats;
};

inline bool characterInstanceIsEnemy(const CharacterInstance& character) {
  return character.type == CharacterTemplateType::ENEMY ||
         character.type == CharacterTemplateType::ENEMY_STATIC;
}

inline void updateCharacterFacingFromMove(CharacterInstance& character, int dx, int dy) {
  if (dx != 0 || dy != 0) {
    character.facing = facingFromMoveDelta(dx, dy);
  }
}

inline void updateCharacterFacingToward(CharacterInstance& character,
                                        int targetX,
                                        int targetY) {
  updateCharacterFacingFromMove(character, targetX - character.x, targetY - character.y);
}

inline bool isCharacterFacingLeft(const CharacterInstance& character) {
  return character.facing == CharacterFacing::Left;
}

/** Copy AI/faction fields from template onto a map character instance. */
void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate);

/** Lookup templateName on the database and apply; returns false if missing. */
bool tryApplyCharacterTemplateToInstance(CharacterInstance& character,
                                         const db::Database& database);

} // namespace model

} // export

namespace model {

void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate) {
  character.type = characterTemplate.type;
  character.label = characterTemplate.label;
  character.behaviorName = characterTemplate.behavior.behaviorName;
  character.visionRadius = characterTemplate.vision.radius;
  character.combatBehaviorTown = characterTemplate.combatBehavior.town;
  character.combatBehaviorCombat = characterTemplate.combatBehavior.combat;
  character.maxHp = characterTemplate.combat.hp;
  character.maxMp = characterTemplate.combat.mp;
  if (character.name.empty()) {
    character.name = characterTemplate.label.empty() ? characterTemplate.name
                                                     : characterTemplate.label;
  }
  if (character.templateName.empty()) {
    character.templateName = characterTemplate.name;
  }
}

bool tryApplyCharacterTemplateToInstance(CharacterInstance& character,
                                         const db::Database& database) {
  if (character.templateName.empty()) {
    return false;
  }
  try {
    const auto& characterTemplate =
        database.getCharacterTemplate(bmin::toStringView(character.templateName));
    applyCharacterTemplateToInstance(character, characterTemplate);
    return true;
  } catch (...) {
    return false;
  }
}

} // namespace model
