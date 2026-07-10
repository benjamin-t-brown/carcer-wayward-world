#pragma once

#include "model/instances/World.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldSetCameraMode : public AbstractAction {
  model::CameraMode cameraMode = model::CameraMode::Follow;

  void act() override {
    if (!state) {
      return;
    }
    state->world.cameraMode = cameraMode;
  }

public:
  explicit WorldSetCameraMode(model::CameraMode _cameraMode)
      : cameraMode(_cameraMode) {}
};

} // namespace actions

} // namespace state
