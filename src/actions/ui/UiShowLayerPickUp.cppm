module;
#include <utility>
#include <optional>

export module carcer.actions.ui.UiShowLayerPickUp;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
  sdl2w::Window* window;
  std::optional<std::pair<int, int>> containerTile;

  void act() override {
    if (containerTile) {
      LayerManagerInterface::showPickUp(window, containerTile->first, containerTile->second);
    } else {
      LayerManagerInterface::showPickUp(window);
    }
  }

public:
  explicit UiShowLayerPickUp(sdl2w::Window* _window) : window(_window) {}

  UiShowLayerPickUp(sdl2w::Window* _window, int containerX, int containerY)
      : window(_window), containerTile(std::make_pair(containerX, containerY)) {}
};

} // namespace actions

} // namespace state

} // export
