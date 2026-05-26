#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerInventoryContext.hpp"
#include "ui/UiElement.h"

namespace ui {
class ObserverShowLayerInventoryContext : public ui::UiEventObserver,
                                          public state::StateManagerInterface {

  sdl2w::Window* window;
  std::string itemName;
  std::string itemId;

public:
  ObserverShowLayerInventoryContext(sdl2w::Window* _window,
                                    std::string_view itemName,
                                    std::string_view itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverShowLayerInventoryContext::onClick " << itemName << " " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiShowLayerInventoryContext(window, itemName, itemId),
        0);
  }
};
} // namespace ui
