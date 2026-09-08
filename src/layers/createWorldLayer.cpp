#include "layers/createWorldLayer.h"

#include "layers/ui/LayerWorld.h"

namespace layers {

bmin::UniquePtr<Layer> createWorldLayer(sdl2w::Window* window, float mapScale) {
  auto* worldLayer = new LayerWorld(window);
  worldLayer->setMapScale(mapScale);
  return bmin::UniquePtr<Layer>(worldLayer);
}

} // namespace layers
