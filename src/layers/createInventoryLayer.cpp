#include "layers/createInventoryLayer.h"

#include "layers/ui/LayerInventory.h"

namespace layers {

Layer* createInventoryLayer(sdl2w::Window* window) {
  return new LayerInventory(window);
}

} // namespace layers
