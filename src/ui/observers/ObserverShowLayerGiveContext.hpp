#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerGiveContext.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverShowLayerGiveContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  String fromCharacterPlayerId;
  String itemId;

public:
  ObserverShowLayerGiveContext(sdl2w::Window* _window,
                               const String& _fromCharacterPlayerId,
                               const String& _itemId)
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
