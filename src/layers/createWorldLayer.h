#pragma once

namespace sdl2w {
class Window;
}

namespace layers {

class Layer;

Layer* createWorldLayer(sdl2w::Window* window, float mapScale = 1.f);

} // namespace layers
