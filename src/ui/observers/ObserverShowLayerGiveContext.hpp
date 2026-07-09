#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerGiveContext.hpp"
#include "ui/UiElement.h"
#include "bmin/String.h"

namespace ui {

class ObserverShowLayerGiveContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

public:
  ObserverShowLayerGiveContext(sdl2w::Window* _window,
                               const bmin::String& _fromCharacterPlayerId,
                               const bmin::String& _itemId)
      : window(_window),
        fromCharacterPlayerId(_fromCharacterPlayerId),
        itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverShowLayerGiveContext::onClick " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiShowLayerGiveContext(
            window, fromCharacterPlayerId, itemId),
        0);
  }
};

} // namespace ui
