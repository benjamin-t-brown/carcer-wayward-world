import carcer.ui.screens;

int main() {
  ui::LayoutRect rect{.x = 1, .y = 2, .width = 3, .height = 4};
  return rect.width == 3 ? 0 : 1;
}
