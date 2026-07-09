#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerDropContext.hpp"
#include "ui/UiElement.h"
#include "bmin/String.h"

namespace ui {

class ObserverShowLayerDropContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverShowLayerDropContext(sdl2w::Window* _window,
                               const bmin::String& _characterPlayerId,
                               const bmin::String& _itemId)
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
