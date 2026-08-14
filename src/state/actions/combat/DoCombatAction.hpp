#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapWalkability.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/DoCombatActionCompletion.hpp"
#include "state/actions/combat/ModifyAP.hpp"
#include "state/actions/combat/MoveCharacter.hpp"
#include "state/actions/combat/PerformMeleeAttack.hpp"
#include "state/actions/combat/PerformSpellCast.hpp"

namespace state {

namespace actions {

struct CombatActionContext {
  bmin::String targetChId;
  bmin::String abilityId;
  model::TileXY targetLoc{};
};

class DoCombatAction : public CombatAction {
  bmin::String chId;
  model::CombatActionType actionType = model::CombatActionType::WAIT;
  CombatActionContext ctx;

  void handleMove() {
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    const auto& actorId = world.combat.activeCharacterId;
    game::ActiveMapOrchestrator orch;
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      return;
    }
    auto dx = ctx.targetLoc.x;
    auto dy = ctx.targetLoc.y;

    model::updateCharacterFacingFromMove(*actor, dx, dy);

    const auto destX = actor->x + dx;
    const auto destY = actor->y + dy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      return;
    }

    if (auto* occupant = orch.findCharacterAt(destX, destY, actorId)) {
      const auto actorIsEnemy = model::isCharacterEnemy(*actor);
      const auto occupantIsEnemy = model::isCharacterEnemy(*occupant);
      if (actorIsEnemy != occupantIsEnemy) {
        insertAction(new PerformMeleeAttack(actorId, occupant->id), 0);
        insertAction(new ModifyAP(actorId, -model::COMBAT_ATTACK_COST), 0);
        return;
      }
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      return;
    }

    insertAction(new MoveCharacter(actorId, ctx.targetLoc.x, ctx.targetLoc.y), 0);
    insertAction(new ModifyAP(actorId, -model::COMBAT_MOVE_COST), 0);
  }

  void handleSpell() {
    model::SpellTargetInfo spellTargetInfo;
    spellTargetInfo.targetCharacterId = ctx.targetChId;
    spellTargetInfo.tileX = ctx.targetLoc.x;
    spellTargetInfo.tileY = ctx.targetLoc.y;
    if (ctx.abilityId.empty()) {
      LOG(INFO) << "DoCombatAction: SPELL with empty spellId" << LOG_ENDL;
      return;
    }
    insertAction(new PerformSpellCast(chId, ctx.abilityId, spellTargetInfo), 0);
    insertAction(nullptr, 150);
  }

  void act() override {
    if (!state || !state->world.combat.active) {
      return;
    }

    const char* actionLabel = "?";
    switch (actionType) {
    case model::CombatActionType::MOVE:
      actionLabel = "MOVE";
      break;
    case model::CombatActionType::SHOOT:
      actionLabel = "SHOOT";
      break;
    case model::CombatActionType::SPELL:
      actionLabel = "SPELL";
      break;
    case model::CombatActionType::WAIT:
      actionLabel = "WAIT";
      break;
    }
    if (actionType == model::CombatActionType::MOVE) {
      LOG(INFO) << "DoCombatAction: " << actionLabel << " for "
                << model::formatCharacterLogLabel(state->world.activeMap,
                                                  state->world.combat.activeCharacterId)
                << " (" << ctx.targetLoc.x << ", " << ctx.targetLoc.y << ")" << LOG_ENDL;
    } else {
      LOG(INFO) << "DoCombatAction: " << actionLabel << " for "
                << model::formatCharacterLogLabel(state->world.activeMap,
                                                  state->world.combat.activeCharacterId)
                << LOG_ENDL;
    }

    state->world.combat.isWaitingForAction = false;

    switch (actionType) {
    case model::CombatActionType::MOVE:
      handleMove();
      break;
    case model::CombatActionType::SPELL:
      handleSpell();
      break;
    case model::CombatActionType::SHOOT:
      insertAction(new DoCombatActionCompletion(), 0);
      break;
    case model::CombatActionType::WAIT: {
      game::ActiveMapOrchestrator orch;
      orch.fetchMapGrid(state->world.activeMap.gridId);
      auto* character = orch.findCharacterById(chId);
      if (character != nullptr) {
        insertAction(new ModifyAP(chId, -character->currentAp), 0);
      }
      break;
    }
    }
    insertAction(new DoCombatActionCompletion(), 0);
  }

public:
  explicit DoCombatAction(const bmin::String& chId, model::CombatActionType _actionType)
      : chId(chId), actionType(_actionType) {}

  DoCombatAction(const bmin::String& chId,
                 model::CombatActionType _actionType,
                 const CombatActionContext& ctx)
      : chId(chId), actionType(_actionType), ctx(ctx) {}
};

} // namespace actions

} // namespace state
