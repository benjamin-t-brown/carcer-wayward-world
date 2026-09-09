#include "ui/helpers/worldCommands.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "game/map/TileDistance.h"
#include "model/Combat.h"
#include "model/instances/Player.h"
#include "state/StateManager.h"
#include "ui/helpers/worldActions.h"
#include "actions/combat/DoCombatAction.hpp"
#include "actions/world/WorldExamineAt.hpp"
#include "actions/world/WorldMovePlayer.hpp"
#include "actions/world/WorldSetActionMode.hpp"
#include "actions/world/WorldTalkAt.hpp"

namespace ui {

bool canPlayerIssueCombatMove(const state::State& state) {
  const auto& world = state.world;
  if (!world.combat.active || !world.combat.isWaitingForAction) {
    return false;
  }
  return model::isPartyMember(state.player, world.combat.activeCharacterId);
}

void enqueueMapMove(state::StateManager& stateManager, int dx, int dy) {
  auto& state = stateManager.getState();
  if (state.world.combat.active) {
    if (!canPlayerIssueCombatMove(state)) {
      return;
    }
    stateManager.enqueueAction(
        state::makeAction<state::actions::DoCombatAction>(
            state.world.combat.activeCharacterId,
            model::CombatActionType::MOVE,
            state::actions::CombatActionContext{.targetLoc = {dx, dy}}),
        0);
    return;
  }
  if (state.world.resolvingTownEnemyAi) {
    return;
  }
  stateManager.enqueueAction(state::makeAction<state::actions::WorldMovePlayer>(dx, dy), 0);
}

void enqueueCombatWait(state::StateManager& stateManager) {
  if (!canPlayerIssueCombatMove(stateManager.getState())) {
    return;
  }
  stateManager.enqueueAction(state::makeAction<state::actions::DoCombatAction>(
                                 stateManager.getState().world.combat.activeCharacterId,
                                 model::CombatActionType::WAIT),
                             0);
}

void ensureCurrentPartyMemberSelection(state::State& state) {
  auto& player = state.player;
  if (player.party.empty()) {
    state.uiState.selectedPartyMemberId.clear();
    return;
  }

  // UI selection only — never tied to map movement / party avatar.
  if (model::playerFindPartyMemberIndexById(player,
                                            state.uiState.selectedPartyMemberId) >= 0) {
    return;
  }
  state.uiState.selectedPartyMemberId = player.party[0].instanceId;
}

void confirmWorldActionAim(state::StateManager& stateManager,
                           sdl2w::Window* window,
                           db::Database* database,
                           int tileX,
                           int tileY) {
  if (stateManager.getState().world.resolvingTownEnemyAi) {
    return;
  }
  const auto actionMode = stateManager.getState().world.actionMode;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    setHeldMoveActive(stateManager, false);
    stateManager.enqueueAction(
        state::makeAction<state::actions::WorldExamineAt>(window, tileX, tileY), 0);
    return;
  }
  if (actionMode == model::WorldActionMode::TALK) {
    setHeldMoveActive(stateManager, false);
    stateManager.enqueueAction(state::makeAction<state::actions::WorldTalkAt>(tileX, tileY), 0);
    return;
  }
  if (actionMode != model::WorldActionMode::SPELL) {
    return;
  }

  auto& state = stateManager.getState();
  auto& world = state.world;
  if (!canPlayerIssueCombatMove(state)) {
    return;
  }
  if (world.pendingSpellId.empty()) {
    return;
  }

  if (database == nullptr) {
    return;
  }
  const auto* spell =
      database->findSpellTemplate(bmin::toStringView(world.pendingSpellId));
  if (spell == nullptr) {
    return;
  }
  const auto* ability =
      database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
  if (ability == nullptr) {
    return;
  }

  const model::CharacterInstance* caster = nullptr;
  for (const auto& character : world.activeMap.characters) {
    if (character.id == world.combat.activeCharacterId) {
      caster = &character;
      break;
    }
  }
  if (caster == nullptr) {
    return;
  }

  const int distance = game::chebyshevDistance(caster->x, caster->y, tileX, tileY);
  if (distance > ability->targetSelect.range) {
    return;
  }

  setHeldMoveActive(stateManager, false);
  stateManager.enqueueAction(
      state::makeAction<state::actions::DoCombatAction>(
          world.combat.activeCharacterId,
          model::CombatActionType::SPELL,
          state::actions::CombatActionContext{.abilityId = world.pendingSpellId,
                                              .targetLoc = {tileX, tileY}}),
      0);
  stateManager.parallelAction(
      state::makeAction<state::actions::WorldSetActionMode>(model::WorldActionMode::NONE), 0);
}

} // namespace ui
