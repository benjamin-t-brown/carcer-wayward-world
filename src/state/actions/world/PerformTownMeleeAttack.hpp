#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileTriggers.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "state/actions/combat/PlaySound.hpp"
#include "state/actions/world/WorldSpawnDamageParticle.hpp"
#include <cstdlib>

namespace state {

namespace actions {

// Applies town-mode party HP change (victim need not be on the active map).
class ModifyPartyMemberHp : public AbstractAction {
  bmin::String instanceId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    model::modifyPartyMemberHp(state->player, instanceId, delta);
  }

public:
  ModifyPartyMemberHp(bmin::String _instanceId, int _delta)
      : instanceId(std::move(_instanceId)), delta(_delta) {}
};

// Town melee with the same swing / particle / reset timing as combat melee.
// Damages a random living party member; FX play on the party avatar tile.
class PerformTownMeleeAttack : public CombatAction {
  bmin::String attackerId;

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

    game::ActiveMapOrchestrator orch;
    auto* attacker = orch.findCharacterById(attackerId);
    auto* avatar = game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (attacker == nullptr || avatar == nullptr) {
      return;
    }

    auto* victim = pickRandomLivingPartyMember(state->player);
    if (victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, avatar->x, avatar->y);

    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertAction(new PlaySound("punch1"), 0);
      insertAction(nullptr, 75);
      insertAction(new ModifyPartyMemberHp(victim->instanceId, -model::COMBAT_MELEE_DAMAGE),
                         0);
      insertAction(new WorldSpawnDamageParticle("splash_attack",
                                                      bmin::toString(model::COMBAT_MELEE_DAMAGE),
                                                      avatar->x,
                                                      avatar->y,
                                                      500),
                         0);
      insertAction(nullptr, 500);
      LOG(INFO) << "TownMeleeAttack: " << attackerId << " hit " << victim->instanceId
                << " for " << model::COMBAT_MELEE_DAMAGE << LOG_ENDL;
    } else {
      insertAction(new PlaySound("whip"), 0);
      insertAction(nullptr, 300);
      LOG(DEBUG) << "TownMeleeAttack: miss by " << attackerId << " vs party member "
                 << victim->instanceId << LOG_ENDL;
    }
    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
  }

public:
  explicit PerformTownMeleeAttack(bmin::String _attackerId)
      : attackerId(std::move(_attackerId)) {}
};

} // namespace actions

} // namespace state
