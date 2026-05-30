#include "LayerPickUp.h"
#include "lib/sdl2w/Logger.h"
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

  auto& style = minipagePickUp->getStyle();
  style.width = std::min(500 / scale, windowWidth / scale);
  style.height = std::min(500 / scale, (windowHeight - 50) / scale);
  style.x = (windowWidth - style.width * scale) / 2;
  style.y = (windowHeight - style.height * scale) / 2;
  style.scale = scale;

  addUiElement(minipagePickUp);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncCurrentPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMember>(
      [this](...) { syncCurrentPartyMember(); });
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
  minipageProps.doneButtonRemoveLayerId = std::string(LAYER_ID);
  minipageProps.partyMemberIndex = player.currentPartyMemberIndex;
  minipageProps.partyMemberSprites.clear();
  for (const auto& member : player.party) {
    minipageProps.partyMemberSprites.push_back(model::characterPlayerGetSprite(member));
  }
  minipageProps.weightText = std::string(TRANSLATE("Carrying")) + " " +
                             std::to_string(carrying) + "/" + std::to_string(maxWeight);
  auto nearbyItems = model::characterGetNearbyItems(*currentPartyMember);
  minipageProps.nearbyItems = nearbyItems;
  minipagePickUp->setProps(minipageProps);
}

} // namespace layers
