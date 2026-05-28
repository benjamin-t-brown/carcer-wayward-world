#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerDropContext.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverShowLayerDropContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  std::string characterPlayerId;
  std::string itemId;

public:
  ObserverShowLayerDropContext(sdl2w::Window* _window,
                               std::string_view _characterPlayerId,
                               std::string_view _itemId)
      : window(_window),
        characterPlayerId(_characterPlayerId),
        itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverShowLayerDropContext::onClick " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiShowLayerDropContext(window, characterPlayerId, itemId),
        0);
  }
};

} // namespace ui
