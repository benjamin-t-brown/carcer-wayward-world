#pragma once

#include "../Layer.h"

namespace layers {

class LayerPickUp : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_pick_up";

  explicit LayerPickUp(sdl2w::Window* _window);
  virtual ~LayerPickUp() = default;

  // void setPickUpItemNames(const bmin::DynArray<bmin::String>& itemNames);
  void syncCurrentPartyMember();
};

} // namespace layers
