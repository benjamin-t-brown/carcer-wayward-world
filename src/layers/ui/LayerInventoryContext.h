#pragma once

#include "layers/Layer.h"

namespace layers {

class LayerInventoryContext : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_inventory_context";
  explicit LayerInventoryContext(sdl2w::Window* _window,
                                 String itemId,
                                 String itemName);
  virtual ~LayerInventoryContext() = default;

  // Override update and render methods
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
