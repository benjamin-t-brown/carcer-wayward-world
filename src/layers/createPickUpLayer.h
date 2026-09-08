#pragma once

#include "bmin/UniquePtr.h"

namespace sdl2w {
class Window;
}

namespace layers {

class Layer;

bmin::UniquePtr<Layer> createPickUpLayer(sdl2w::Window* window);

} // namespace layers
