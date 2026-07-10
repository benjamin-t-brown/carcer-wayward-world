#pragma once

#include "../Layer.h"

namespace layers {

class LayerWorld : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_world";

  explicit LayerWorld(sdl2w::Window* _window);
  virtual ~LayerWorld() = default;

  void syncFromState();
};

} // namespace layers
