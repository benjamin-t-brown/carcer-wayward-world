#include "layers/createPickUpLayer.h"

#include "layers/ui/LayerPickUp.h"

namespace layers {

Layer* createPickUpLayer(sdl2w::Window* window) {
  return new LayerPickUp(window);
}

} // namespace layers
