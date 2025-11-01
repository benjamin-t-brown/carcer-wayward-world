#pragma once

#include "../Layer.h"

namespace layers {

class LayerContextMenuInventoryItem : public Layer {
public:
  explicit LayerContextMenuInventoryItem(sdl2w::Window* _window,
                                         std::string itemId,
                                         std::string itemName);
  virtual ~LayerContextMenuInventoryItem() = default;

  // Override update and render methods
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

