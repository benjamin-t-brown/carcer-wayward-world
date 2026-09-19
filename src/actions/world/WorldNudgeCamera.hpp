#pragma once

#include "game/map/Camera.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class WorldNudgeCamera : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldNudgeCamera; }
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camera.camX += dx * game::kCameraTileWidth;
    state->world.camera.camY += dy * game::kCameraTileHeight;
  }

public:
  WorldNudgeCamera(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
