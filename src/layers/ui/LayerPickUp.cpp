#include "LayerPickUp.h"
#include "lib/StringUtil.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "state/actions/ui/UiSetCurrentPartyMember.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/minipages/MinipagePickUp.h"

namespace layers {

LayerPickUp::LayerPickUp(sdl2w::Window* _window) : Layer(_window, LAYER_ID) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto minipagePickUp = new ui::MinipagePickUp(window);
  minipagePickUp->setId("minipagePickUp");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  const int width = std::min(static_cast<int>(500 / scale),
                             static_cast<int>(windowWidth / scale));
  const int height = std::min(static_cast<int>(500 / scale),
                              static_cast<int>((windowHeight - 50) / scale));
  minipagePickUp->setPos((windowWidth - width * scale) / 2,
                         (windowHeight - height * scale) / 2);
  minipagePickUp->setScale(scale);
  auto minipageInitProps = minipagePickUp->getProps();
  minipageInitProps.width = width;
  minipageInitProps.height = height;
  minipagePickUp->setProps(minipageInitProps);

  addUiElement(minipagePickUp);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncCurrentPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMember>(
      [this](auto&, auto&) { syncCurrentPartyMember(); });
}

void LayerPickUp::syncCurrentPartyMember() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto database = getDatabase();

  auto& player = stateManager->getState().player;
  auto currentPartyMember =
      model::playerFindPartyMemberByIndex(player, player.currentPartyMemberIndex);

  if (!currentPartyMember) {
    LOG(ERROR) << "LayerPickUp::LayerPickUp: currentPartyMember is nullptr" << LOG_ENDL;
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
  auto nearbyItems = model::characterGetNearbyItems(*currentPartyMember);
  minipageProps.nearbyItems = nearbyItems;
  minipagePickUp->setProps(minipageProps);
}

} // namespace layers
