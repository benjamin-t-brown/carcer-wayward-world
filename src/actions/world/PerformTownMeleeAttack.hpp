#pragma once

#include "actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/ModifyPartyMemberHp.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileTriggers.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include <cstdlib>

namespace state {

namespace actions {

// Town melee with the same swing / particle / reset timing as combat melee.
// Damages a random living party member; FX play on the party avatar tile.
class PerformTownMeleeAttack : public AbstractAction {
  bmin::String attackerId;

  ActionEvent getEvent() const override { return ActionEvent::PerformTownMeleeAttack; }

  static model::CharacterPlayer* pickRandomLivingPartyMember(model::Player& player) {
    bmin::DynArray<model::CharacterPlayer*> living;
    for (size_t i = 0; i < player.party.size(); i++) {
      if (player.party[i].currentHp > 0) {
        living.pushBack(&player.party[i]);
      }
    }
    if (living.empty()) {
      return nullptr;
    }
    const auto index = static_cast<size_t>(std::rand() % static_cast<int>(living.size()));
    return living[index];
  }

  void act() override {
    if (!state) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    auto* attacker = orch.findCharacterById(attackerId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (attacker == nullptr || avatar == nullptr) {
      return;
    }

    auto* victim = pickRandomLivingPartyMember(state->player);
    if (victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, avatar->x, avatar->y);

    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(attackerId, 1), 0);

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertAction(state::makeAction<PlaySound>("hit_punch1"), 0);
      insertAction(nullptr, 75);
      insertAction(state::makeAction<ModifyPartyMemberHp>(victim->instanceId,
                                                          -model::COMBAT_MELEE_DAMAGE),
                   0);
      insertAction(state::makeAction<WorldSpawnDamageParticle>(
                       "splash_attack",
                       bmin::toString(model::COMBAT_MELEE_DAMAGE),
                       avatar->x,
                       avatar->y,
                       500),
                   0);
      insertAction(nullptr, 500);
      LOG(INFO) << "TownMeleeAttack: " << attackerId << " hit " << victim->instanceId
                << " for " << model::COMBAT_MELEE_DAMAGE << LOG_ENDL;
    } else {
      insertAction(state::makeAction<PlaySound>("whip"), 0);
      insertAction(nullptr, 300);
      LOG(DEBUG) << "TownMeleeAttack: miss by " << attackerId << " vs party member "
                 << victim->instanceId << LOG_ENDL;
    }
    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(attackerId, 0), 0);
  }

public:
  explicit PerformTownMeleeAttack(bmin::String _attackerId)
      : attackerId(std::move(_attackerId)) {}
};

} // namespace actions

} // namespace state
