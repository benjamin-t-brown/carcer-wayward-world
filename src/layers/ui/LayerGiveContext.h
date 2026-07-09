#pragma once

#include "layers/Layer.h"

namespace layers {

class LayerGiveContext : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_give_context";
  explicit LayerGiveContext(sdl2w::Window* _window,
                            String fromCharacterPlayerId,
                            String itemId);
  ~LayerGiveContext() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
