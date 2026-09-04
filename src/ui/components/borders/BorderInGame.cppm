module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.BorderInGame;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.ui.elements;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/components/borders/BorderInGame.h ---
namespace ui {

struct BorderInGameProps {
  int titleHeight = 44;
  int outsetBorderSize = 4;
  float actionButtonsScale = 1.f;
};

// BorderInGame component - base class for in-game border layouts using OutsetRectangle
// elements. Uses Position, Size, Scale from BaseStyle
class BorderInGame : public UiElement {
public:
  static constexpr int ACTION_BUTTON_SIZE = 32;

protected:
  virtual const BorderInGameProps& inGameProps() const = 0;

  int scaledWidth() const;
  int scaledHeight() const;
  void addOutsetRect(int x, int y, int width, int height);

public:
  BorderInGame(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGame() override = default;

  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getTitleDims() const;

  virtual const std::pair<int, int> getContentAreaLocation() const = 0;
  virtual const std::pair<int, int> getContentDims() const = 0;
  virtual const std::pair<int, int> getPartyMemberAreaLocation() const = 0;
  virtual const std::pair<int, int> getActionButtonsAreaLocation() const = 0;

  void render(int dt) override;
};

} // namespace ui

} // export

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
  rectangle->setPos(x, y);
  rectangle->setScale(style.scale);
  rectangle->setProps(OutsetRectangleProps{
      .width = width,
      .height = height,
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
