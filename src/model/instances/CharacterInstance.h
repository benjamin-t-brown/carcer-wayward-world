#pragma once

#include "bmin/String.h"
#include "model/templates/CharacterTemplate.h"

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
  // Original map spawn tile; used to persist defeated map-placed characters after movement.
  int spawnX = -1;
  int spawnY = -1;
  // Combat runtime (meaningful while world.combat.active).
  int currentAp = 0;
  int currentHp = 0; // enemies only; party HP lives on CharacterPlayer
  // Cached from CharacterTemplate.combat.hp (enemies/NPCs); party HP lives on CharacterPlayer.
  int maxHp = 0;
  // True once currentHp has been set from combat (distinguishes 0 HP from uninitialized).
  bool hpInitialized = false;
  // Transient pose offset (e.g. weapon swing frame); reset via CharacterSetSpriteIndexOffset.
  int spriteIndexOffset = 0;
  // Default art faces right; left uses horizontal flip at render time.
  CharacterFacing facing = CharacterFacing::Right;
  // Map AI: set when IMMOBILE_UNTIL_ENEMY_SPOTTED spots the party (not persisted).
  bool agitated = false;

  // Cached from CharacterTemplate at spawn / active-map hoist (for AI without DB lookups).
  CharacterTemplateType type = CharacterTemplateType::TOWNSPERSON;
  bmin::String label;
  bmin::String behaviorName;
  int visionRadius = 0;
  CombatBehaviorName combatBehaviorTown = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combatBehaviorCombat = CombatBehaviorName::SEEK_AND_MELEE;
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

inline void updateCharacterFacingToward(CharacterInstance& character, int targetX, int targetY) {
  updateCharacterFacingFromMove(character, targetX - character.x, targetY - character.y);
}

inline bool isCharacterFacingLeft(const CharacterInstance& character) {
  return character.facing == CharacterFacing::Left;
}

} // namespace model
