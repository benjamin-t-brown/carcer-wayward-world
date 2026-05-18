#pragma once

#include "../UiElement.h"
#include "ui/components/BorderModalSmall.h"

namespace ui {

// struct BorderModalStandardProps {
//   int topLeftSquareSize = 78;
//   int leftBorderWidth = 16;
//   int borderSize = 4;
//   int closeButtonPadding = 6;
//   int subtitleYOffset = 40;
// };

// BorderModalStandard component - renders a specific border layout using OutsetRectangle
// elements Uses Position, Size, Scale from BaseStyle
class BorderModalStandard : public BorderModalSmall {
private:
  // BorderModalStandardProps props;

public:
  static const int BOTTOM_BORDER_HEIGHT = 10;
  BorderModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalStandard() override = default;

  // void setProps(const BorderModalStandardProps& _props);
  // BorderModalStandardProps& getProps();
  // const BorderModalStandardProps& getProps() const;

  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getContentLoc() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
