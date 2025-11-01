#pragma once

#include "../Layer.h"

namespace layers {

class LayerContextMenuPickUpItem : public Layer {
public:
  explicit LayerContextMenuPickUpItem(sdl2w::Window* _window,
                                      std::string itemId,
                                      std::string itemName);
  virtual ~LayerContextMenuPickUpItem() = default;

  // Override update and render methods
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers


