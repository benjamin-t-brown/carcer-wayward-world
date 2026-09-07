import carcer.ui.screens;

int main() {
  ui::LayoutRect rect{.x = 1, .y = 2, .width = 3, .height = 4};
  layers::LayerState state = layers::LayerState::ON;
  return rect.width == 3 && state == layers::LayerState::ON ? 0 : 1;
}
