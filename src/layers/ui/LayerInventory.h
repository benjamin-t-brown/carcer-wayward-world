#pragma once

#include "../Layer.h"

namespace layers {

class LayerInventory : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_inventory";

  explicit LayerInventory(sdl2w::Window* _window);
  virtual ~LayerInventory() = default;

  void onKeyDown(std::string_view key, int keyCode) override;

  void syncInventoryPartyMember();
};

} // namespace layers
