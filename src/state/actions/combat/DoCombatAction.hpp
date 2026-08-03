#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/Combat.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapWalkability.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/DoCombatActionCompletion.hpp"
#include "state/actions/combat/ModifyAP.hpp"
#include "state/actions/combat/MoveCharacter.hpp"
#include "state/actions/combat/PerformMeleeAttack.hpp"

namespace state {

namespace actions {

class DoCombatAction : public CombatAction {
  model::CombatActionType actionType = model::CombatActionType::WAIT;
  int moveDx = 0;
  int moveDy = 0;
  model::TileXY shootTarget{};
  model::CombatSpellTarget spellTarget;

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
      insertCombatAction(new DoCombatActionCompletion(), 0);
      return;
    }

    model::updateCharacterFacingFromMove(*actor, moveDx, moveDy);

    const auto destX = actor->x + moveDx;
    const auto destY = actor->y + moveDy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      insertCombatAction(new DoCombatActionCompletion(), 0);
      return;
    }

    if (auto* occupant = orch.findCharacterAt(destX, destY, actorId)) {
      const auto actorIsEnemy = model::isCharacterEnemy(*actor, *database);
      const auto occupantIsEnemy = model::isCharacterEnemy(*occupant, *database);
      if (actorIsEnemy != occupantIsEnemy) {
        insertCombatAction(new PerformMeleeAttack(actorId, occupant->id), 0);
        insertCombatAction(new ModifyAP(actorId, -model::COMBAT_ATTACK_COST), 0);
        insertCombatAction(new DoCombatActionCompletion(), 0);
        return;
      }
      insertCombatAction(new DoCombatActionCompletion(), 0);
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      insertCombatAction(new DoCombatActionCompletion(), 0);
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      insertCombatAction(new DoCombatActionCompletion(), 0);
      return;
    }

    insertCombatAction(new MoveCharacter(actorId, moveDx, moveDy), 0);
    insertCombatAction(new ModifyAP(actorId, -model::COMBAT_MOVE_COST), 0);
    insertCombatAction(new DoCombatActionCompletion(), 0);
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
                << " (" << moveDx << ", " << moveDy << ")" << LOG_ENDL;
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
    case model::CombatActionType::SHOOT:
      insertCombatAction(new DoCombatActionCompletion(), 0);
      break;
    case model::CombatActionType::WAIT: {
      game::ActiveMapOrchestrator orch;
      auto* character = orch.findCharacterById(state->world.combat.activeCharacterId);
      if (character != nullptr) {
        character->currentAp = 0;
      }
      insertCombatAction(new DoCombatActionCompletion(), 0);
      break;
    }
    }
  }

public:
  explicit DoCombatAction(model::CombatActionType _actionType) : actionType(_actionType) {}

  DoCombatAction(model::CombatActionType _actionType, int dx, int dy)
      : actionType(_actionType), moveDx(dx), moveDy(dy) {}

  DoCombatAction(model::CombatActionType _actionType, model::TileXY target)
      : actionType(_actionType), shootTarget(target) {}

  DoCombatAction(model::CombatActionType _actionType, model::CombatSpellTarget target)
      : actionType(_actionType), spellTarget(std::move(target)) {}
};

} // namespace actions

} // namespace state
