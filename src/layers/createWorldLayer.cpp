#include "layers/createWorldLayer.h"

#include "layers/ui/LayerWorld.h"

namespace layers {

Layer* createWorldLayer(sdl2w::Window* window, float mapScale) {
  auto* layer = new LayerWorld(window);
  layer->setMapScale(mapScale);
  return layer;
}

} // namespace layers
