export module carcer.actions.world.WorldSetCameraMode;
export import carcer.state;

export {

namespace state {

namespace actions {

class WorldSetCameraMode : public AbstractAction {
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

} // export
