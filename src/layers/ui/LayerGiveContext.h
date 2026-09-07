#pragma once

#include "layers/UiLayer.h"

namespace layers {

class LayerGiveContext : public UiLayer {
public:
  constexpr static std::string_view LAYER_ID = "layer_give_context";
  explicit LayerGiveContext(sdl2w::Window* _window,
                            bmin::String fromCharacterPlayerId,
                            bmin::String itemId);
  ~LayerGiveContext() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
