#pragma once

#include "../UiElement.h"

namespace ui {

// BorderModalSmall component - renders a smaller border layout using OutsetRectangle
// elements Uses Position, Size, Scale from BaseStyle
class BorderModalSmall : public UiElement {
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
