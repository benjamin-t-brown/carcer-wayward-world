#pragma once

#include "../../UiElement.h"

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
