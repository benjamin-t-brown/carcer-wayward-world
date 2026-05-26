#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerPickupContext.hpp"
#include "ui/UiElement.h"

namespace ui {
class ObserverShowLayerPickUpContext : public ui::UiEventObserver,
                                       public state::StateManagerInterface {

  sdl2w::Window* window;
  model::ItemInstance item;

public:
  ObserverShowLayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item)
      : window(_window), item(item) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverShowLayerPickUpContext::onClick " << item.itemName << " "
              << item.id << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiShowLayerPickupContext(window, item),
        0);
  }
};
} // namespace ui
