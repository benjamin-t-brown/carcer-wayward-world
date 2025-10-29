#pragma once

#include "../UiElement.h"

namespace ui {

// BorderModalStandard component - renders a specific border layout using OutsetRectangle elements
// Uses Position, Size, Scale from BaseStyle
class BorderModalStandard : public UiElement {
private:
  // Layout constants
  static const int TOP_LEFT_SQUARE_SIZE = 78;
  static const int LEFT_BORDER_WIDTH = 16;
  static const int BORDER_SIZE = 4;
  // static const int BOTTOM_BORDER_WIDTH = 4;
  static const int CLOSE_BUTTON_PADDING = 6;
  // static const int TITLE_PADDING = 10;
  static const int SUBTITLE_Y_OFFSET = 40;

public:
  BorderModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalStandard() override = default;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconLocation(int iconWidth, int IconHeight) const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getSubTitleLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
