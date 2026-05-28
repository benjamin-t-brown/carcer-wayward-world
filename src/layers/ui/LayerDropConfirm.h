#pragma once

#include "layers/Layer.h"

namespace layers {

class LayerDropConfirm : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_drop_confirm";
  explicit LayerDropConfirm(sdl2w::Window* _window,
                            std::string characterPlayerId,
                            std::string itemId);
  ~LayerDropConfirm() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
