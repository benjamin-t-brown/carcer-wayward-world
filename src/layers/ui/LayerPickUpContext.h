#pragma once

#include "layers/UiLayer.h"

namespace layers {

class LayerPickUpContext : public UiLayer {
private:
  model::ItemInstance item;

public:
  constexpr static std::string_view LAYER_ID = "layer_pick_up_context";
  explicit LayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item);
  virtual ~LayerPickUpContext() = default;

  // Override update and render methods
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
