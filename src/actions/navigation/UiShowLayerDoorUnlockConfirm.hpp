#pragma once

namespace sdl2w { class Window; }

#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerDoorUnlockConfirm : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiShowLayerDoorUnlockConfirm;
  }
  int worldX = 0;
  int worldY = 0;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::DoorUnlockConfirm,
                                  .x = worldX,
                                  .y = worldY,
                                  .hasPosition = true});
  }

public:
  UiShowLayerDoorUnlockConfirm(sdl2w::Window* /*window*/, int _worldX, int _worldY)
      : worldX(_worldX), worldY(_worldY) {}
};

} // namespace actions

} // namespace state
