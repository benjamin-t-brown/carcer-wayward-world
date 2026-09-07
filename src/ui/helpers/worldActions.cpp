#include "ui/helpers/worldActions.h"

#include "state/StateManager.h"
#include "actions/combat/EndCombat.hpp"
#include "actions/combat/StartCombat.hpp"
#include "actions/navigation/UiShowLayerInventory.hpp"
#include "actions/navigation/UiShowLayerMagic.hpp"
#include "actions/navigation/UiShowLayerPickUp.hpp"
#include "actions/navigation/UiShowLayerSpellCast.hpp"
#include "actions/navigation/heldMove/UiUpdateHeldMove.hpp"
#include "actions/world/WorldInteractAt.hpp"
#include "actions/world/WorldSetActionMode.hpp"

namespace ui {

void setHeldMoveActive(state::StateManager& stateManager, bool isActive) {
  auto nextHeldMove = stateManager.getState().uiState.heldMove;
  nextHeldMove.isActive = isActive;
  stateManager.pllAction(stateManager.getActionData(),
                         new state::actions::UiUpdateHeldMove(nextHeldMove),
                         0);
}

void cancelCurrentWorldActionMode(state::StateManager& stateManager) {
  // Cancels EXAMINE / TALK / SPELL aim (NONE clears aim tile + pendingSpellId).
  if (stateManager.getState().world.actionMode == model::WorldActionMode::NONE) {
    return;
  }
  setHeldMoveActive(stateManager, false);
  stateManager.pllAction(
      stateManager.getActionData(),
      new state::actions::WorldSetActionMode(model::WorldActionMode::NONE),
      0);
}

void showMagicSetupLayer(state::StateManager& stateManager, sdl2w::Window* window) {
  if (!window) {
    return;
  }
  setHeldMoveActive(stateManager, false);
  stateManager.enqueueAction(
      stateManager.getActionData(), new state::actions::UiShowLayerMagic(window), 0);
}

void showSpellCastLayer(state::StateManager& stateManager, sdl2w::Window* window) {
  if (!window) {
    return;
  }
  auto& state = stateManager.getState();
  auto& world = stateManager.getState().world;

  auto chId = stateManager.getState().world.combat.activeCharacterId;
  if (!world.combat.active || chId.empty()) {
    chId = state.uiState.selectedPartyMemberId;
  }
  setHeldMoveActive(stateManager, false);
  stateManager.enqueueAction(stateManager.getActionData(),
                             new state::actions::UiShowLayerSpellCast(window, chId),
                             0);
}

void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType,
                         sdl2w::Window* window) {
  if (stateManager.getState().world.resolvingTownEnemyAi) {
    return;
  }

  const auto currentMode = stateManager.getState().world.actionMode;

  switch (worldActionType) {
  case state::WorldActionType::EXAMINE:
    setHeldMoveActive(stateManager, false);
    if (currentMode == model::WorldActionMode::EXAMINE) {
      cancelCurrentWorldActionMode(stateManager);
      break;
    }
    stateManager.pllAction(
        stateManager.getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::EXAMINE),
        0);
    break;
  case state::WorldActionType::TALK:
    setHeldMoveActive(stateManager, false);
    if (currentMode == model::WorldActionMode::TALK) {
      cancelCurrentWorldActionMode(stateManager);
      break;
    }
    stateManager.pllAction(
        stateManager.getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::TALK),
        0);
    break;
  case state::WorldActionType::INVENTORY:
    if (!window) {
      break;
    }
    setHeldMoveActive(stateManager, false);
    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::UiShowLayerInventory(window),
                               0);
    break;
  case state::WorldActionType::ABILITY:
    // Combat Ability button → spell-cast list; non-combat → magic setup.
    // Keyboard `r` uses showMagicSetupLayer directly so combat still opens LayerMagic.
    if (stateManager.getState().world.combat.active) {
      showSpellCastLayer(stateManager, window);
    } else {
      showMagicSetupLayer(stateManager, window);
    }
    break;
  case state::WorldActionType::GET:
    if (!window) {
      break;
    }
    setHeldMoveActive(stateManager, false);
    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::UiShowLayerPickUp(window), 0);
    break;
  case state::WorldActionType::INTERACT:
    setHeldMoveActive(stateManager, false);
    cancelCurrentWorldActionMode(stateManager);
    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::WorldInteractAt(), 0);
    break;
  case state::WorldActionType::START_FIGHT:
    setHeldMoveActive(stateManager, false);
    cancelCurrentWorldActionMode(stateManager);
    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::StartCombat(), 0);
    break;
  case state::WorldActionType::END_FIGHT:
    setHeldMoveActive(stateManager, false);
    cancelCurrentWorldActionMode(stateManager);
    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::EndCombat(), 0);
    break;
  default:
    break;
  }
}

} // namespace ui
