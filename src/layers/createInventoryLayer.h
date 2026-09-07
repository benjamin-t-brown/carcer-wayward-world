#pragma once

namespace sdl2w {
class Window;
}

namespace layers {

class Layer;

Layer* createInventoryLayer(sdl2w::Window* window);

} // namespace layers
