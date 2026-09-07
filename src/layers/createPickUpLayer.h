#pragma once

namespace sdl2w {
class Window;
}

namespace layers {

class Layer;

Layer* createPickUpLayer(sdl2w::Window* window);

} // namespace layers
