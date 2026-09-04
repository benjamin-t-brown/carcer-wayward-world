module;
#include <utility>
#include <cstddef>
#include <optional>

export module carcer.actions.ui.layers:UiShowLayerPickUp;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
  std::optional<std::pair<int, int>> containerTile;

  void act() override {
    if (!state) {
      return;
    }
    LayerRequest request{.id = LayerId::PickUp};
    if (containerTile) {
      request.x = containerTile->first;
      request.y = containerTile->second;
    }
    pushLayerRequest(*state, std::move(request));
  }

public:
  explicit UiShowLayerPickUp(sdl2w::Window* /*_window*/) {}

  UiShowLayerPickUp(sdl2w::Window* /*_window*/, int containerX, int containerY)
      : containerTile(std::make_pair(containerX, containerY)) {}
};

} // namespace actions

} // namespace state

} // export
