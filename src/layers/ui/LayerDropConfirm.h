#pragma once

#include "layers/UiLayer.h"

namespace layers {

class LayerDropConfirm : public UiLayer {
public:
  constexpr static std::string_view LAYER_ID = "layer_drop_confirm";
  explicit LayerDropConfirm(sdl2w::Window* _window,
                            bmin::String characterPlayerId,
                            bmin::String itemId);
  ~LayerDropConfirm() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
