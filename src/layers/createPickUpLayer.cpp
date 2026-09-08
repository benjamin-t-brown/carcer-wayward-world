#include "layers/createPickUpLayer.h"

#include "layers/ui/LayerPickUp.h"

namespace layers {

bmin::UniquePtr<Layer> createPickUpLayer(sdl2w::Window* window) {
  return bmin::UniquePtr<Layer>(new LayerPickUp(window));
}

} // namespace layers
