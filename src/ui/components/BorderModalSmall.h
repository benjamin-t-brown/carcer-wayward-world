#pragma once

#include "../UiElement.h"

namespace ui {

// BorderModalSmall component - renders a smaller border layout using OutsetRectangle
// elements Uses Position, Size, Scale from BaseStyle
class BorderModalSmall : public UiElement {
private:
  // Layout constants
  static const int TOP_LEFT_SQUARE_SIZE = 78;
  static const int BORDER_WIDTH = 4;
  static const int CLOSE_BUTTON_PADDING = 6;
  // static const int TITLE_PADDING = 10;
  static const int SUBTITLE_Y_OFFSET = 40;

public:
  BorderModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalSmall() override = default;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconLocation(int iconWidth, int IconHeight) const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getSubTitleLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void build() override;
  void render() override;
};

} // namespace ui
