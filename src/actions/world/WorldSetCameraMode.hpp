#pragma once

#include "model/instances/World.hpp"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class WorldSetCameraMode : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSetCameraMode; }
  model::CameraMode cameraMode = model::CameraMode::Follow;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camera.cameraMode = cameraMode;
  }

public:
  explicit WorldSetCameraMode(model::CameraMode _cameraMode)
      : cameraMode(_cameraMode) {}
};

} // namespace actions

} // namespace state
