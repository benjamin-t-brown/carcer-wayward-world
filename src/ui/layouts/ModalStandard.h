#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/helpers/modalLayoutFit.h"
#include "bmin/String.h"

namespace ui {

// ModalStandard layout properties
struct ModalStandardProps {
  // For LayoutFit::CappedCentered (default), width/height are window dims.
  // For LayoutFit::FullBleed, width/height are the modal size as-is.
  int width = 0;
  int height = 0;
  LayoutFit layoutFit = LayoutFit::CappedCentered;
  SDL_Color contentBackgroundColor = Colors::White;
  bmin::String decorationSprite = "";
  bmin::String iconSprite;
  // Scales headerHeight (base 80) and the portrait that fits inside the top-left
  // OutsetRectangle well (headerHeight - 2 * outset border).
  float portraitScale = 1.f;
};

// ModalStandard layout - renders a modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalStandard : public UiElement {
private:
  ModalStandardProps props;

public:
  ModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalStandard() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalStandardProps& _props);
  ModalStandardProps& getProps();
  const ModalStandardProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();

  const std::pair<int, int> getSubTitleDims();
  const std::pair<int, int> getSubTitleLocation();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
