#pragma once

#include "model/instances/CharacterInstance.hpp"
#include "model/templates/Abilities.hpp"
#include <optional>

namespace db {
class Database;
}

namespace model {
struct Player;
}

namespace game {

struct ResolvedShootAbility {
  model::AbilityTemplate ability;
};

struct ShootAuxSpendResult {
  bool ok = false;
  /** HP to remove from the shooter. Applied by the caller via ModifyHP. */
  int hpCost = 0;
};

/** First equipped WEAPON_RANGED (main hand, then off hand) with a resolved ability.
 *  Applies the item's damage overrides. Empty when the character has no shoot weapon. */
std::optional<ResolvedShootAbility> resolveEquippedShootAbility(
    const model::Player& player,
    const model::CharacterInstance& shooter,
    const db::Database& database);

/** True when the ability's aux cost can be paid right now. Cooldown cannot be paid. */
bool canPayShootAuxCost(const model::Player& player,
                        const model::CharacterInstance& shooter,
                        const model::AbilityTemplate& ability);

/** Equipped ranged weapon, AP still above zero, and a payable aux cost.
 *  The shot spends COMBAT_SHOOT_COST and may drive AP negative. */
bool canStartShoot(const model::Player& player,
                   const model::CharacterInstance& shooter,
                   const db::Database& database);

/** Spends ammo or mana immediately. HP is reported in hpCost and not applied here.
 *  Nothing is spent when the cost cannot be paid. */
ShootAuxSpendResult spendShootAuxCost(model::Player& player,
                                      model::CharacterInstance& shooter,
                                      const model::AbilityTemplate& ability);

} // namespace game
