module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <algorithm>

export module carcer.ui.layouts.ModalStandard;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.SdlPixels;
export import carcer.ui.UiElement;
export import carcer.ui.helpers;
import sdl2w;
import carcer.ui.BorderModalSmall;
import carcer.ui.BorderModalStandard;
import carcer.ui.elements;
import carcer.ui.uiUtils;
#include "macros.h"

export {

// --- from ui/layouts/ModalStandard.h ---
// IWYU pragma: keep

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

} // export

namespace ui {

ModalStandard::ModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ModalStandard::setProps(const ModalStandardProps& _props) {
  props = _props;
  build();
}

ModalStandardProps& ModalStandard::getProps() { return props; }

const ModalStandardProps& ModalStandard::getProps() const { return props; }

const std::pair<int, int> ModalStandard::getSubTitleDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getSubTitleDims();
}

const std::pair<int, int> ModalStandard::getSubTitleLocation() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getSubTitleLocation();
}

const std::pair<int, int> ModalStandard::getContentDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentDims();
}

const std::pair<int, int> ModalStandard::getContentLocation() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentLocation();
}

void ModalStandard::build() {
  removeChildById("border");
  removeChildById("closeButton");
  removeChildById("headerIcon");

  if (props.width > 0 && props.height > 0) {
    if (props.layoutFit == LayoutFit::CappedCentered) {
      const auto rect = computeCappedCenteredRect(props.width, props.height);
      style.x = rect.x;
      style.y = rect.y;
      style.width = rect.width;
      style.height = rect.height;
    } else {
      style.width = props.width;
      style.height = props.height;
    }
  }

  // Create border element
  auto border = new BorderModalStandard(window, this);
  border->setId("border");
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  // portraitScale grows headerHeight; portrait fits the inner outset well.
  constexpr int kBaseHeaderHeight = 80;
  const float portraitScale = props.portraitScale > 0.f ? props.portraitScale : 1.f;
  const int headerHeight = static_cast<int>(kBaseHeaderHeight * portraitScale);
  const int outsetBorder = OutsetRectangleProps{}.borderSize; // 4 → inner well -8
  const int portraitFitSize = std::max(1, headerHeight - outsetBorder * 2);
  border->setProps(BorderModalSmallProps{
      .width = style.width,
      .height = style.height,
      .headerHeight = headerHeight,
      .iconSize = portraitFitSize,
      .borderWidth = 2,
  });

  // Insert border at the beginning
  addChild(border);

  // Get close button location before moving border
  auto [closeX, closeY] = border->getCloseButtonLocation();

  auto modalClose = new ButtonClose(window, this);
  modalClose->setId("closeButton");
  modalClose->setPos(closeX, closeY);
  modalClose->setScale(style.scale);
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  addChild(modalClose);

  if (!props.iconSprite.empty()) {
    auto* border = dynamic_cast<BorderModalStandard*>(getChildById("border"));
    if (border != nullptr) {
      // drawSprite always uses the sprite's native w/h (params.w/h are ignored), so the
      // Quad texture must match the sprite or the image is clipped (e.g. 52px portraits
      // into a 32px texture looked shifted up-left).
      const auto& sprite =
          window->getStore().getSprite(bmin::toStringView(props.iconSprite));
      const int spriteW = std::max(1, sprite.w);
      const int spriteH = std::max(1, sprite.h);
      // Inner area of the top-left OutsetRectangle: headerHeight - 2 * borderSize.
      const int fitSize = border->getProps().iconSize;
      const float fitScale =
          static_cast<float>(fitSize) / static_cast<float>(std::max(spriteW, spriteH));
      const float drawScale = fitScale * style.scale;
      const int screenW = static_cast<int>(spriteW * drawScale);
      const int screenH = static_cast<int>(spriteH * drawScale);

      auto [centerX, centerY] = border->getIconSectionCenter();

      auto icon = new Quad(window, this);
      icon->setId("headerIcon");
      icon->setPos(centerX - screenW / 2, centerY - screenH / 2);
      icon->setScale(drawScale);
      icon->setProps(QuadProps{
          .width = spriteW,
          .height = spriteH,
          .bgSprite = props.iconSprite,
      });
      addChild(icon);
    }
  }

  // TODO decoration sprite
}

void ModalStandard::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    _titleElement->setPos(titleX, titleY - titleHeight / 2);
    _titleElement->setId("title");
    addChild(_titleElement);
  }
}

UiElement* ModalStandard::getTitleElement() { return getChildById("title"); }

UiElement* ModalStandard::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalStandard::render(int dt) { UiElement::render(dt); }

} // namespace ui
