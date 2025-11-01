#pragma once

#include "../Layer.h"

namespace layers {

class LayerInventoryItemContextMenu : public Layer {
public:
  explicit LayerInventoryItemContextMenu(sdl2w::Window* _window,
                                         std::string itemId,
                                         std::string itemName);
  virtual ~LayerInventoryItemContextMenu() = default;

  // Override update and render methods
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
