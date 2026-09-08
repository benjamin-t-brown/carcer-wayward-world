#pragma once

#include "bmin/String.h"
#include "state/StateManager.h"
#include "actions/navigation/UiShowLayerEquipRunes.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverShowLayerEquipRunes : public ui::UiEventObserver,
                                    public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String characterPlayerId;

public:
  ObserverShowLayerEquipRunes(sdl2w::Window* _window,
                              const bmin::String& _characterPlayerId)
      : window(_window), characterPlayerId(_characterPlayerId) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    LOG(INFO) << "ObserverShowLayerEquipRunes::onClick character="
              << characterPlayerId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || characterPlayerId.empty()) {
      return;
    }
    stateManager->enqueueAction(state::makeAction<state::actions::UiShowLayerEquipRunes>(window, characterPlayerId),
        0);
  }
};

} // namespace ui
