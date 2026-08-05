#include "LayerPickUp.h"
#include "game/map/MapPickup.h"
#include "game/map/TileTriggers.h"
#include "lib/StringUtil.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "state/actions/ui/UiPickUpItem.hpp"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "state/actions/ui/UiSetCurrentPartyMember.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/minipages/MinipagePickUp.h"

namespace layers {

ui::ButtonModal* LayerPickUp::findDoneButton() {
  auto* minipagePickUp = getUiElement<ui::MinipagePickUp>("minipagePickUp");
  if (!minipagePickUp) {
    return nullptr;
  }
  auto* modal = minipagePickUp->getChildById("modal");
  if (!modal) {
    return nullptr;
  }
  auto* buttonGroup = modal->getChildById("buttonGroup");
  if (!buttonGroup) {
    return nullptr;
  }
  return dynamic_cast<ui::ButtonModal*>(buttonGroup->getChildById("buttonGroupButton_0"));
}

void LayerPickUp::beginCloseWithDonePress() {
  if (isClosing) {
    return;
  }
  isClosing = true;
  donePressElapsedMs = 0;
  if (auto* doneButton = findDoneButton()) {
    doneButton->isActive = true;
  }
}

LayerPickUp::LayerPickUp(sdl2w::Window* _window) : Layer(_window, LAYER_ID) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto minipagePickUp = new ui::MinipagePickUp(window);
  minipagePickUp->setId("minipagePickUp");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  // Window dims + ModalSmall default CappedCentered (see ui::computeCappedCenteredRect).
  minipagePickUp->setPos(0, 0);
  minipagePickUp->setScale(scale);
  auto minipageInitProps = minipagePickUp->getProps();
  minipageInitProps.width = static_cast<int>(windowWidth / scale);
  minipageInitProps.height = static_cast<int>(windowHeight / scale);
  minipagePickUp->setProps(minipageInitProps);

  addUiElement(minipagePickUp);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncCurrentPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMember>(
      [this](auto&, auto&) { syncCurrentPartyMember(); });
  subscribeAction<state::actions::UiPickUpItem>(
      [this](auto&, auto&) { syncCurrentPartyMember(); });
}

LayerPickUp::LayerPickUp(sdl2w::Window* _window, int containerX, int containerY)
    : LayerPickUp(_window) {
  containerTile = std::make_pair(containerX, containerY);
  syncCurrentPartyMember();
}

void LayerPickUp::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON || isClosing) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }

  if (const auto partyIndex = ui::getPartyMemberIndexFromKey(key)) {
    if (*partyIndex <
        static_cast<int>(stateManager->getState().player.party.size())) {
      stateManager->enqueueAction(
          stateManager->getActionData(),
          new state::actions::UiSetCurrentPartyMember(*partyIndex),
          0);
    }
    return;
  }

  if (const auto itemIndex = ui::getPickUpItemIndexFromKey(key)) {
    auto* minipagePickUp = getUiElement<ui::MinipagePickUp>("minipagePickUp");
    if (minipagePickUp &&
        *itemIndex <
            static_cast<int>(minipagePickUp->getProps().nearbyItems.size())) {
      const auto& item = minipagePickUp->getProps().nearbyItems[*itemIndex];
      stateManager->enqueueAction(stateManager->getActionData(),
                                  new state::actions::UiPickUpItem(item.id),
                                  0);
    }
    return;
  }

  if (ui::isCancelActionKey(key) || ui::isConfirmActionKey(key)) {
    beginCloseWithDonePress();
  }
}


void LayerPickUp::syncCurrentPartyMember() {
  if (isClosing) {
    return;
  }
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto database = getDatabase();
  if (!stateManager || !database) {
    remove();
    return;
  }

  auto& state = stateManager->getState();
  auto& player = state.player;
  auto currentPartyMember =
      model::playerFindPartyMemberByIndex(player, player.currentPartyMemberIndex);

  if (!currentPartyMember) {
    LOG(ERROR) << "LayerPickUp::syncCurrentPartyMember: party member is nullptr"
               << LOG_ENDL;
    remove();
    return;
  }

  auto minipagePickUp = getUiElement<ui::MinipagePickUp>("minipagePickUp");
  if (!minipagePickUp) {
    LOG(ERROR) << "LayerPickUp::syncCurrentPartyMember: minipagePickUp is nullptr"
               << LOG_ENDL;
    return;
  }

  const int carrying = model::characterGetWeightCarrying(*currentPartyMember, database);
  const int maxWeight = model::characterGetWeightCapacity(*currentPartyMember);

  auto minipageProps = minipagePickUp->getProps();
  minipageProps.doneButtonRemoveLayerId = strutil::fromStringView(LAYER_ID);
  minipageProps.partyMemberIndex = player.currentPartyMemberIndex;
  minipageProps.partyMemberSprites.clear();
  for (const auto& member : player.party) {
    minipageProps.partyMemberSprites.pushBack(model::characterPlayerGetSprite(member));
  }
  minipageProps.weightText = bmin::String(TRANSLATE("Carrying")) + " " +
                             bmin::toString(carrying) + "/" + bmin::toString(maxWeight);

  minipageProps.nearbyItems.clear();
  if (containerTile) {
    minipageProps.titleText = TRANSLATE("Container");
    minipageProps.nearbyItems = game::collectItemsAtActiveMapTile(
        state.world.activeMap, containerTile->first, containerTile->second);
    if (minipageProps.nearbyItems.empty()) {
      minipageProps.statusText = TRANSLATE("Nothing inside.");
    } else {
      minipageProps.statusText.clear();
    }
  } else {
    minipageProps.titleText = TRANSLATE("Pick Up");
    if (const auto* avatar = game::findDropCharacterOnActiveMap(
            state.world.activeMap, player, currentPartyMember->instanceId)) {
      minipageProps.nearbyItems = game::collectItemsWithinPickupRange(
          state.world.activeMap, *avatar, game::PICKUP_PATH_RANGE, *database);
    }
    if (minipageProps.nearbyItems.empty()) {
      minipageProps.statusText = TRANSLATE("No items nearby.");
    } else {
      minipageProps.statusText.clear();
    }
  }

  minipagePickUp->setProps(minipageProps);
}

void LayerPickUp::update(int deltaTime) {
  Layer::update(deltaTime);
  if (!isClosing || closeEnqueued) {
    return;
  }

  // Keep Done visually pressed while the close delay runs.
  if (auto* doneButton = findDoneButton()) {
    doneButton->isActive = true;
  }

  donePressElapsedMs += deltaTime;
  if (donePressElapsedMs < donePressDurationMs) {
    return;
  }

  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  closeEnqueued = true;
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(bmin::String(LAYER_ID.data(), LAYER_ID.size())),
      0);
}


} // namespace layers
