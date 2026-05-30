#include "BorderInGame.h"
#include "../../elements/OutsetRectangle.h"

namespace ui {

BorderInGame::BorderInGame(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

int BorderInGame::scaledWidth() const {
  return static_cast<int>(style.width * style.scale);
}

int BorderInGame::scaledHeight() const {
  return static_cast<int>(style.height * style.scale);
}

void BorderInGame::addOutsetRect(int x, int y, int width, int height) {
  auto rectangle = new OutsetRectangle(window, this);
  auto& rectStyle = rectangle->getStyle();
  rectStyle.x = x;
  rectStyle.y = y;
  rectStyle.width = width;
  rectStyle.height = height;
  rectStyle.scale = style.scale;
  rectangle->setProps(OutsetRectangleProps{
      .borderSize = inGameProps().outsetBorderSize,
  });
  addChild(rectangle);
}

const std::pair<int, int> BorderInGame::getTitleLocation() const {
  const auto& props = inGameProps();
  int titleX = style.x + props.outsetBorderSize * style.scale;
  int titleY = style.y + props.outsetBorderSize * style.scale;
  return {titleX, titleY};
}

const std::pair<int, int> BorderInGame::getTitleDims() const {
  const auto& props = inGameProps();
  return {style.width * style.scale - props.outsetBorderSize * 2 * style.scale,
          props.titleHeight * style.scale - props.outsetBorderSize * 2 * style.scale};
}

void BorderInGame::render(int dt) { UiElement::render(dt); }

} // namespace ui
