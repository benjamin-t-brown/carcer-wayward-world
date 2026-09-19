#pragma once

#include "bmin/DynArray.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"

namespace db {
class Database;
}

namespace model {
struct CharacterInstance;
struct CharacterStats;
struct Player;
} // namespace model

namespace game {

inline constexpr const char* kMeleeAttackDefaultName = "MELEE_ATTACK_DEFAULT";

struct ResolvedMeleeAbility {
  model::AbilityTemplate ability;
  bool offHand = false;
  bool unarmed = false;
};

bmin::DynArray<ResolvedMeleeAbility> resolveMeleeAttackAbilities(
    model::Player& player,
    const model::CharacterInstance& attacker,
    const db::Database& database);

int weaponMasteryBonus(const model::CharacterStats& stats,
                       const model::AbilityAttack& attack,
                       bool unarmed);

int meleeTargetArmorClass(const model::CharacterStats& defenderStats);

bool meleeAttackHits(int d20Roll, int attackBonus, int targetArmorClass);

bool rollMeleeAttackHit(model::AttackClass attackClass,
                        int attackBonus,
                        int targetArmorClass,
                        bool applyOffHandPenalty);

} // namespace game
