#pragma once

#include "../UiElement.h"

namespace ui {

// BorderModalStandard component - renders a specific border layout using OutsetRectangle elements
// Uses Position, Size, Scale from BaseStyle
class BorderModalStandard : public UiElement {
public:
  BorderModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalStandard() override = default;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render() override;
};

} // namespace ui
