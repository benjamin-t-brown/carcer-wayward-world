#pragma once

#include "layers/UiLayer.h"

namespace layers {

class LayerSpecialItemContext : public UiLayer {
public:
  constexpr static std::string_view LAYER_ID = "layer_special_item_context";
  explicit LayerSpecialItemContext(sdl2w::Window* _window,
                                   bmin::String itemTemplateName,
                                   bmin::String quantityText);
  ~LayerSpecialItemContext() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
