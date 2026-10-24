#pragma once

#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "model/stats/CharacterStats.h"
#include "model/templates/CharacterTemplate.h"

namespace model {

enum class CharacterFacing { Right, Left };

// Clamp a tile delta to one 8-way step. (0, 0) stays (0, 0).
inline void unitStepFromDelta(int dx, int dy, int& outDx, int& outDy) {
  outDx = dx > 0 ? 1 : (dx < 0 ? -1 : 0);
  outDy = dy > 0 ? 1 : (dy < 0 ? -1 : 0);
}

// Up, up-right, right, down-right → Right; left, up-left, down-left, down → Left.
inline CharacterFacing facingFromMoveDelta(int dx, int dy) {
  if (dx < 0 || (dx == 0 && dy > 0)) {
    return CharacterFacing::Left;
  }
  return CharacterFacing::Right;
}

struct AppliedStatusEffect {
  bmin::String statusEffectName;
  int remainingTurns = 0;
};

// Map-entity character (player avatar, future NPCs). Distinct from CharacterPlayer
// (party chrome). Position is tile coords in map space, same as TileInstance.
struct CharacterInstance {
  bmin::String id;
  bmin::String name;
  bmin::String templateName;
  int x = 0;
  int y = 0;
  // Floor / map layer (same space as ActiveMap::mapLayer and placement.l).
  int mapLayer = 0;
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
  // Last non-zero facing step (one 8-way tile). Waits leave this unchanged.
  // Default matches default facing Right.
  int lastMoveDx = 1;
  int lastMoveDy = 0;
  // Map AI: spotting / contagion (not persisted).
  bool agitated = false;
  // From map placement; characters with the same non-empty group agitate together.
  bmin::String agitationGroup;

  // Cached from CharacterTemplate at spawn / active-map hoist (for AI without DB
  // lookups).
  CharacterTemplateType type = CharacterTemplateType::TOWNSPERSON;
  bmin::String label;
  bmin::String behaviorName;
  int visionRadius = 0;
  CombatBehaviorName combatBehaviorTown = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combatBehaviorCombat = CombatBehaviorName::SEEK_AND_MELEE;
  CharacterTemplateCombatSpellAi combatSpellAi;
  // Mode chosen for the current combat turn (see resolveCombatTurnBehavior).
  CombatBehaviorName combatBehaviorThisTurn = CombatBehaviorName::SEEK_AND_MELEE;
  // combatTurnsTaken value when combatBehaviorThisTurn was last chosen.
  int combatBehaviorChosenTurn = -1;
  // Incremented at the start of each of this actor's combat turns.
  int combatTurnsTaken = 0;
  CharacterStats stats;
  bmin::DynArray<AppliedStatusEffect> statusEffects;
};

inline bool characterInstanceIsEnemy(const CharacterInstance& character) {
  return character.type == CharacterTemplateType::ENEMY ||
         character.type == CharacterTemplateType::ENEMY_STATIC;
}

inline void updateCharacterFacingFromMove(CharacterInstance& character, int dx, int dy) {
  if (dx == 0 && dy == 0) {
    return;
  }
  int unitDx = 0;
  int unitDy = 0;
  unitStepFromDelta(dx, dy, unitDx, unitDy);
  character.facing = facingFromMoveDelta(unitDx, unitDy);
  character.lastMoveDx = unitDx;
  character.lastMoveDy = unitDy;
}

inline void updateCharacterFacingToward(CharacterInstance& character,
                                        int targetX,
                                        int targetY) {
  updateCharacterFacingFromMove(character, targetX - character.x, targetY - character.y);
}

inline bool isCharacterFacingLeft(const CharacterInstance& character) {
  return character.facing == CharacterFacing::Left;
}

} // namespace model
