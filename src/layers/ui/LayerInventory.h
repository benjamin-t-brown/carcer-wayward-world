#pragma once

#include "../Layer.h"

namespace layers {

class LayerInventory : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_inventory";

  explicit LayerInventory(sdl2w::Window* _window);
  virtual ~LayerInventory() = default;

  void syncCurrentPartyMember();
};

} // namespace layers
