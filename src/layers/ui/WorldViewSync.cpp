#include "WorldViewSync.h"

#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include "bmin/StringInterop.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "layers/LayerManager.h"
#include "layers/UiLayer.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "state/LayerRequest.h"
#include "state/StateManager.h"
#include "actions/navigation/UiSetSelectedPartyMemberId.hpp"
#include "actions/world/WorldNudgeCamera.hpp"
#include "ui/components/InGameTitleBar.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/helpers/worldCommands.h"
#include "ui/layouts/InGameLayout.h"
#include "ui/observers/ActionObserver.hpp"
#include "ui/observers/ObserverCancelWorldActionMode.hpp"
#include "ui/observers/ObserverWorldAction.hpp"

namespace layers {

bool WorldViewSync::assertInterfaces() const {
  return hasStateManager() && hasDatabase();
}

void WorldViewSync::setWorldActionTypes(model::TurnMode turnMode,
                                        bmin::DynArray<state::WorldActionType>& dest) {
  auto copyActionTypes = [](bmin::DynArray<state::WorldActionType>& dest,
                            const auto& source) {
    dest.clear();
    for (const auto& type : source) {
      dest.pushBack(type);
    }
  };

  const state::WorldActionUiState worldActionUiState;
  switch (turnMode) {
  case model::TurnMode::TURN_OUTDOOR:
    copyActionTypes(dest, worldActionUiState.outdoorModeActionTypes);
    break;
  case model::TurnMode::TURN_COMBAT:
    copyActionTypes(dest, worldActionUiState.townModeFightActionTypes);
    break;
  case model::TurnMode::TURN_TOWN:
  default:
    copyActionTypes(dest, worldActionUiState.townModeActionTypes);
    break;
  }
}

void WorldViewSync::attachWorldActionObservers(ui::InGameLayout* inGameLayout) {
  if (!inGameLayout) {
    return;
  }

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    button->addEventObserver(bmin::UniquePtr<ui::UiEventObserver>(new ui::ObserverWorldAction(
        stateManager, button->getProps().worldActionType, owner.getWindow())));
  }
}

void WorldViewSync::attachPartyMemberObservers(ui::InGameLayout* inGameLayout) {
  if (!inGameLayout) {
    return;
  }

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto* chList = inGameLayout->getChildById("chList");
  if (!chList) {
    return;
  }
  auto* list = chList->getChildById("list");
  if (!list) {
    return;
  }

  const auto& party = stateManager->getState().player.party;
  auto& children = list->getChildren();
  for (size_t i = 0; i < children.size() && i < party.size(); i++) {
    // Party member switching is locked while combat is active, so the observer
    // builds no action mid-combat.
    children[i]->addEventObserver(bmin::UniquePtr<ui::UiEventObserver>(
        new ui::ActionObserver(
            [id = party[i].instanceId](state::StateManager& sm)
                -> bmin::UniquePtr<state::AbstractAction> {
              if (sm.getState().world.combat.active) {
                return bmin::UniquePtr<state::AbstractAction>();
              }
              return state::makeAction<state::actions::UiSetSelectedPartyMemberId>(
                  id);
            })));
  }
}

void WorldViewSync::syncWorldActionModeHighlight() {
  auto* inGameLayout = owner.getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  auto* manager = owner.getLayerManager();
  const auto isOpen = [&](state::LayerId id) {
    return manager != nullptr && manager->containsLayer(id);
  };
  const bool inventoryOpen = isOpen(state::LayerId::Inventory);
  const bool magicOpen = isOpen(state::LayerId::Magic);
  const bool spellCastOpen =
      isOpen(state::LayerId::SpellCast) || isOpen(state::LayerId::SpellAllyTarget);
  const bool pickUpOpen = isOpen(state::LayerId::PickUp);
  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    const auto actionType = button->getProps().worldActionType;
    const bool modeSelected =
        (actionType == state::WorldActionType::EXAMINE &&
         actionMode == model::WorldActionMode::EXAMINE) ||
        (actionType == state::WorldActionType::TALK &&
         actionMode == model::WorldActionMode::TALK) ||
        (actionType == state::WorldActionType::INVENTORY && inventoryOpen) ||
        (actionType == state::WorldActionType::ABILITY && (magicOpen || spellCastOpen)) ||
        (actionType == state::WorldActionType::GET && pickUpOpen);
    if (button->isModeSelected != modeSelected) {
      button->isModeSelected = modeSelected;
    }
  }
}

void WorldViewSync::syncActionModeCancelButton() {
  auto* inGameLayout = owner.getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  bmin::String modeLabel;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    modeLabel = TRANSLATE("Examine");
  } else if (actionMode == model::WorldActionMode::TALK) {
    modeLabel = TRANSLATE("Talk");
  } else if (actionMode == model::WorldActionMode::SPELL) {
    modeLabel = TRANSLATE("Cast Spell");
  }

  const bool shouldShow = !modeLabel.empty();
  auto* existingCancel = inGameLayout->getChildById("actionModeCancel");
  auto* existingLabel =
      dynamic_cast<ui::TextLine*>(inGameLayout->getChildById("actionModeLabel"));
  if (shouldShow && existingCancel && existingLabel) {
    const auto& labelProps = existingLabel->getProps();
    if (!labelProps.textBlocks.empty() && labelProps.textBlocks[0].text == modeLabel) {
      return;
    }
  } else if (!shouldShow && !existingCancel && !existingLabel) {
    return;
  }

  inGameLayout->setActionModeCancelVisible(shouldShow, modeLabel);
  if (shouldShow) {
    if (auto* cancelButton = inGameLayout->getChildById("actionModeCancel")) {
      cancelButton->addEventObserver(
          bmin::UniquePtr<ui::UiEventObserver>(new ui::ObserverCancelWorldActionMode(getStateManager())));
    }
  }
}

void WorldViewSync::syncSpellAimCameraButtons() {
  auto* inGameLayout = owner.getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto& world = getStateManager()->getState().world;
  const bool shouldShow =
      world.combat.active && world.actionMode == model::WorldActionMode::SPELL;
  auto* existing = inGameLayout->getChildById("spellAimCameraUp");
  if (shouldShow == (existing != nullptr)) {
    return;
  }

  inGameLayout->setSpellAimCameraButtonsVisible(shouldShow);
  if (!shouldShow) {
    return;
  }

  struct PanButton {
    const char* id;
    int dx;
    int dy;
  };
  const PanButton panButtons[] = {
      {"spellAimCameraUp", 0, -1},
      {"spellAimCameraDown", 0, 1},
      {"spellAimCameraLeft", -1, 0},
      {"spellAimCameraRight", 1, 0},
  };
  for (const auto& pan : panButtons) {
    if (auto* button = inGameLayout->getChildById(pan.id)) {
      button->addEventObserver(
          ui::makeActionObserver<state::actions::WorldNudgeCamera>(pan.dx, pan.dy));
    }
  }
}

void WorldViewSync::syncActionModeCursor() {
  auto* window = owner.getWindow();
  if (!window || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  auto cursor = sdl2w::SystemCursor::Arrow;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    cursor = sdl2w::SystemCursor::Crosshair;
  } else if (actionMode == model::WorldActionMode::TALK) {
    cursor = sdl2w::SystemCursor::Hand;
  }
  if (!window->hasColorCursor() && window->getSystemCursor() == cursor) {
    return;
  }
  window->setSystemCursor(cursor);
}

void WorldViewSync::syncCombatTitleBar() {
  auto* inGameLayout = owner.getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    return;
  }
  auto* titleBar = dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement());
  if (!titleBar) {
    return;
  }

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& state = stateManager->getState();
  auto& world = state.world;
  game::ActiveMapOrchestrator activeMap(
      world.activeMap, state.mapInstances, getDatabase());
  auto titleProps = titleBar->getProps();
  const bool showAp = world.combat.active;
  int ap = 0;
  if (showAp) {
    if (const auto* character =
            activeMap.findCharacterById(world.combat.activeCharacterId)) {
      ap = character->currentAp;
    }
  }
  if (titleProps.showAp != showAp || titleProps.ap != ap) {
    titleProps.showAp = showAp;
    titleProps.ap = ap;
    titleBar->setProps(titleProps);
  }
}

void WorldViewSync::refresh() {
  auto* inGameLayout = owner.getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    LOG(ERROR) << "WorldViewSync::refresh: inGameLayout is nullptr" << LOG_ENDL;
    return;
  }

  auto* stateManager = getStateManager();
  auto& state = stateManager->getState();
  auto& player = state.player;
  auto& world = state.world;
  game::ActiveMapOrchestrator activeMap(
      world.activeMap, state.mapInstances, getDatabase());

  ui::ensureCurrentPartyMemberSelection(state);

  const int selectedIndex =
      model::playerFindPartyMemberIndexById(player, state.uiState.selectedPartyMemberId);

  auto layoutProps = inGameLayout->getProps();
  setWorldActionTypes(state.turnMode, layoutProps.worldActionTypes);
  layoutProps.partyMembers.clear();
  layoutProps.selectedPartyMemberIndex = selectedIndex >= 0 ? selectedIndex : 0;
  for (int i = 0; i < static_cast<int>(player.party.size()); i++) {
    const auto& member = player.party[i];
    ui::ChCompactInfoProps entry;
    entry.characterSpriteName = model::characterPlayerGetSprite(member);
    entry.hp = member.currentHp;
    entry.mana = member.currentMp;
    entry.isSelected = (i == layoutProps.selectedPartyMemberIndex);
    if (const auto* mapCharacter = activeMap.findCharacterById(member.instanceId)) {
      if (auto* database = getDatabase()) {
        for (size_t s = 0; s < mapCharacter->statusEffects.size(); s++) {
          const auto* statusTemplate = database->findStatusEffectTemplate(
              bmin::toStringView(mapCharacter->statusEffects[s].statusEffectName));
          if (statusTemplate != nullptr && !statusTemplate->icon.empty()) {
            entry.statusEffectSpriteNames.pushBack(statusTemplate->icon);
          }
        }
      }
    }
    layoutProps.partyMembers.pushBack(entry);
  }
  inGameLayout->setProps(layoutProps);
  attachWorldActionObservers(inGameLayout);
  attachPartyMemberObservers(inGameLayout);
  syncWorldActionModeHighlight();
  syncActionModeCancelButton();
  syncSpellAimCameraButtons();
  syncActionModeCursor();

  if (auto* titleBar =
          dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement())) {
    auto titleProps = titleBar->getProps();
    titleProps.title = bmin::String("World");
    // day/ap placeholders until those fields live on State
    titleProps.day = 0;
    titleProps.food = player.food;
    titleProps.ap = 0;
    if (world.combat.active) {
      if (const auto* character =
              activeMap.findCharacterById(world.combat.activeCharacterId)) {
        titleProps.ap = character->currentAp;
      }
    }
    titleProps.showAp = world.combat.active;
    titleBar->setProps(titleProps);
  }
}

} // namespace layers
