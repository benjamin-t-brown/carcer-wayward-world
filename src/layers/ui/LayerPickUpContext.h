#pragma once

#include "layers/Layer.h"

namespace layers {

class LayerPickUpContext : public Layer {
private:
  model::ItemInstance item;

public:
  explicit LayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item);
  virtual ~LayerPickUpContext() = default;

  // Override update and render methods
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
