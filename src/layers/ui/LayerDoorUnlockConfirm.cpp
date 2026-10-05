#include "LayerDoorUnlockConfirm.h"
#include "actions/navigation/UiConfirmDoorUnlock.hpp"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "bmin/StringInterop.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapWalkability.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "ui/components/ConfirmModal.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/helpers/uiSounds.h"
#include "ui/popups/PopupDoorUnlockConfirm.h"

namespace layers {

bmin::String LayerDoorUnlockConfirm::messageForBump(const game::ClosedDoorBumpInfo& bump) {
  switch (bump.outcome) {
  case game::ClosedDoorBumpOutcome::ConfirmToolUnlock:
    return TRANSLATE("This door is locked.\n\nIt requires ") +
           bmin::toString(bump.toolsRequired) +
           TRANSLATE(" tools to unlock.\n\nWould you like to do so?");
  case game::ClosedDoorBumpOutcome::InfoInsufficientTools:
    return TRANSLATE("This door is locked.\n\nIt requires ") +
           bmin::toString(bump.toolsRequired) +
           TRANSLATE(" tools to unlock.\n\nYou do not have enough to do so.");
  case game::ClosedDoorBumpOutcome::InfoMissingKey:
    return TRANSLATE("This door is locked and you do not have the key to open it.");
  case game::ClosedDoorBumpOutcome::ConfirmKeyUnlock:
    return TRANSLATE(
        "This door is locked, but you have the key to unlock it.\n\nWould you like to do so now?");
  case game::ClosedDoorBumpOutcome::OpenImmediateSilent:
  case game::ClosedDoorBumpOutcome::OpenImmediateLockpick:
  case game::ClosedDoorBumpOutcome::OpenImmediateBash:
    break;
  }
  return {};
}

bool LayerDoorUnlockConfirm::isConfirmOutcome(game::ClosedDoorBumpOutcome outcome) {
  return outcome == game::ClosedDoorBumpOutcome::ConfirmToolUnlock ||
         outcome == game::ClosedDoorBumpOutcome::ConfirmKeyUnlock;
}

LayerDoorUnlockConfirm::LayerDoorUnlockConfirm(sdl2w::Window* _window,
                                               int worldX,
                                               int worldY)
    : UiLayer(_window, LAYER_ID), worldX(worldX), worldY(worldY) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto* stateManager = getStateManager();
  auto* database = getDatabase();
  if (!stateManager || !database) {
    LOG(ERROR) << "LayerDoorUnlockConfirm: stateManager or database is nullptr"
               << LOG_ENDL;
    remove();
    return;
  }

  auto& state = stateManager->getState();
  if (state.player.party.empty()) {
    LOG(ERROR) << "LayerDoorUnlockConfirm: party is empty" << LOG_ENDL;
    remove();
    return;
  }

  game::ActiveMapOrchestrator orch(state.world.activeMap, state.mapInstances, database);
  auto* map = orch.getMapInstanceAt(worldX, worldY);
  const auto local = orch.activeMapCoordToInstanceCoord(worldX, worldY);
  if (!map || !local.valid) {
    LOG(ERROR) << "LayerDoorUnlockConfirm: map tile not found" << LOG_ENDL;
    remove();
    return;
  }
  map->tileLayerNumber = state.world.activeMap.mapLayer;

  auto* door = game::findClosedDoorAt(*map, local.x, local.y, *database);
  if (!door) {
    LOG(ERROR) << "LayerDoorUnlockConfirm: closed door not found" << LOG_ENDL;
    remove();
    return;
  }

  const auto bump = game::classifyClosedDoorBump(
      *door, state.player.party[0], state.specialEventStorage, *database);
  if (!isConfirmOutcome(bump.outcome) &&
      bump.outcome != game::ClosedDoorBumpOutcome::InfoInsufficientTools &&
      bump.outcome != game::ClosedDoorBumpOutcome::InfoMissingKey) {
    LOG(ERROR) << "LayerDoorUnlockConfirm: unexpected immediate bump outcome"
               << LOG_ENDL;
    remove();
    return;
  }

  const auto message = messageForBump(bump);
  if (message.empty()) {
    LOG(ERROR) << "LayerDoorUnlockConfirm: empty message" << LOG_ENDL;
    remove();
    return;
  }

  dismissOnly = !isConfirmOutcome(bump.outcome);

  auto [windowWidth, windowHeight] = window->getDims();

  auto popup = new ui::PopupDoorUnlockConfirm(window, nullptr);
  popup->setId("popupDoorUnlockConfirm");

  ui::PopupDoorUnlockConfirmProps popupProps;
  popupProps.worldX = worldX;
  popupProps.worldY = worldY;
  popupProps.message = message;
  popupProps.dismissOnly = dismissOnly;
  popup->setProps(popupProps);

  popup->setScale(1.f);
  auto [popupW, popupH] = popup->getDims();
  popup->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popup->build();

  addUiElement(bmin::UniquePtr<ui::UiElement>(popup));

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));
}

ui::ButtonModal* LayerDoorUnlockConfirm::buttonAtIndex(int index) {
  auto* popup = getUiElement<ui::PopupDoorUnlockConfirm>("popupDoorUnlockConfirm");
  if (!popup) {
    return nullptr;
  }
  auto* modal = dynamic_cast<ui::ConfirmModal*>(popup->getChildById("confirmModal"));
  if (!modal) {
    return nullptr;
  }
  auto* buttonGroup = modal->getButtonGroup();
  if (!buttonGroup) {
    return nullptr;
  }
  const bmin::String buttonId = "buttonGroupButton_" + bmin::toString(index);
  return dynamic_cast<ui::ButtonModal*>(
      buttonGroup->getChildById(bmin::toStringView(buttonId)));
}

void LayerDoorUnlockConfirm::enqueueDismiss() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiRemoveLayer>(state::LayerId::DoorUnlockConfirm),
      0);
}

void LayerDoorUnlockConfirm::enqueueConfirm() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiConfirmDoorUnlock>(worldX, worldY), 0);
}

void LayerDoorUnlockConfirm::beginKeyboardButtonPress(
    int buttonIndex, std::function<void()> onComplete) {
  if (keyboardFlash.isBusy()) {
    return;
  }
  keyboardFlash.begin(
      [this, buttonIndex]() -> bool* {
        if (auto* button = buttonAtIndex(buttonIndex)) {
          return &button->isActive;
        }
        return nullptr;
      },
      std::move(onComplete),
      window);
}

void LayerDoorUnlockConfirm::beginKeyboardDismissPress() {
  // dismissOnly: single Okay at index 0. Yes/No: No is index 0.
  beginKeyboardButtonPress(0, [this]() { enqueueDismiss(); });
}

void LayerDoorUnlockConfirm::beginKeyboardConfirmPress() {
  if (dismissOnly) {
    beginKeyboardDismissPress();
    return;
  }
  // Yes is index 1.
  beginKeyboardButtonPress(1, [this]() { enqueueConfirm(); });
}

void LayerDoorUnlockConfirm::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON || keyboardFlash.isBusy()) {
    return;
  }

  if (ui::isCancelActionKey(key)) {
    beginKeyboardDismissPress();
    return;
  }

  if (ui::KeyboardPressFlash::isConfirmKey(key)) {
    beginKeyboardConfirmPress();
    return;
  }

  if (dismissOnly) {
    return;
  }

  // 1 = Yes (confirm), 2 = No (dismiss). Button order stays No then Yes.
  if (const auto choiceIndex = ui::KeyboardPressFlash::choiceIndexFromKey(key)) {
    if (*choiceIndex == 0) {
      beginKeyboardConfirmPress();
    } else if (*choiceIndex == 1) {
      beginKeyboardDismissPress();
    }
  }
}

void LayerDoorUnlockConfirm::update(int deltaTime) {
  keyboardFlash.update(deltaTime);
  UiLayer::update(deltaTime);
}

void LayerDoorUnlockConfirm::render(int deltaTime) { UiLayer::render(deltaTime); }

} // namespace layers
