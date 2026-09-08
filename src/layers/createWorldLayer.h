#pragma once

#include "bmin/UniquePtr.h"

namespace sdl2w {
class Window;
}

namespace layers {

class Layer;

bmin::UniquePtr<Layer> createWorldLayer(sdl2w::Window* window, float mapScale = 1.f);

} // namespace layers
