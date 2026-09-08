#include "layers/createInventoryLayer.h"

#include "layers/ui/LayerInventory.h"

namespace layers {

bmin::UniquePtr<Layer> createInventoryLayer(sdl2w::Window* window) {
  return bmin::UniquePtr<Layer>(new LayerInventory(window));
}

} // namespace layers
