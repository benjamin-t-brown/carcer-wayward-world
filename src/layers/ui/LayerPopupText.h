#pragma once

#include "bmin/String.h"
#include "layers/Layer.h"
#include <string_view>

namespace layers {

class LayerPopupText : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_popup_text";

  explicit LayerPopupText(sdl2w::Window* _window,
                          bmin::String title,
                          bmin::String text);
  ~LayerPopupText() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
