import carcer.ui.core;

int main() {
  ui::BaseStyle style{.x = 3, .y = 4, .width = 5, .height = 6};
  ui::TextFontProps font;
  font.fontColor = ui::Colors::White;
  return style.x + style.y == 7 ? 0 : 1;
}
