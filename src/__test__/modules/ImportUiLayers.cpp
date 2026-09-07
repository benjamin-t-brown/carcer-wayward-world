import carcer.ui.layers;
import sdl2w;

int main() {
  auto* (*factory)(sdl2w::Window*, float) = &layers::createWorldLayer;
  layers::LayerState state = layers::LayerState::ON;
  return factory != nullptr && state == layers::LayerState::ON ? 0 : 1;
}
