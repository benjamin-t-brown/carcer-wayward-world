#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldSetCamera : public AbstractAction {
  int camX = 0;
  int camY = 0;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camX = camX;
    state->world.camY = camY;
  }

public:
  WorldSetCamera(int _camX, int _camY) : camX(_camX), camY(_camY) {}
};

} // namespace actions

} // namespace state
