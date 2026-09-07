#pragma once

namespace sdl2w { class Window; }

#include "state/AbstractAction.h"
#include "state/State.h"
#include <optional>

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPickUp; }
  std::optional<std::pair<int, int>> containerTile;

  void act() override {
    if (!state) {
      return;
    }
    LayerRequest request{.id = LayerId::PickUp};
    if (containerTile) {
      request.x = containerTile->first;
      request.y = containerTile->second;
      request.hasPosition = true;
    }
    pushLayerRequest(*state, std::move(request));
  }

public:
  explicit UiShowLayerPickUp(sdl2w::Window* /*window*/) {}

  UiShowLayerPickUp(sdl2w::Window* /*window*/, int containerX, int containerY)
      : containerTile(std::make_pair(containerX, containerY)) {}
};

} // namespace actions

} // namespace state
