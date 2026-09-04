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

export module carcer.ui.elements:TextBanner;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
import :OutsetRectangle;
import :TextLine;
#include "macros.h"

export {

// --- from ui/elements/TextBanner.h ---
namespace ui {

enum class TextBannerCorner { LEFT_TOP, LEFT_BOTTOM, RIGHT_TOP, RIGHT_BOTTOM };

struct TextBannerProps {
  std::pair<int, int> location = {0, 0};
  std::pair<int, int> dims = {0, 0};
  TextBannerCorner corner = TextBannerCorner::LEFT_TOP;
  bmin::String text;
  SDL_Color backgroundColor = Colors::Grey;
  int padding = 6;
  int outsetBorderSize = 0;
  FontFamily fontFamily = FontFamily::TEXT_BOLD;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::White;
};

// TextBanner element - text with an outset rectangle background, placed in a corner
// of a container defined by location and dims props.
class TextBanner : public UiElement {
private:
  TextBannerProps props;

  std::pair<int, int> measureTextScaled() const;
  std::pair<int, int> calculateBannerScreenPosition(int bannerScaledWidth,
                                                      int bannerScaledHeight) const;

public:
  TextBanner(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextBanner() override = default;

  void setProps(const TextBannerProps& _props);
  TextBannerProps& getProps();
  const TextBannerProps& getProps() const;

  void setPos(int x, int y) override;
  void setScale(float scale) override;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

TextBanner::TextBanner(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT_BOLD);
  props.fontFamily = font.fontFamily;
  props.fontSize = font.fontSize;
  props.fontColor = font.fontColor;
}

void TextBanner::setProps(const TextBannerProps& _props) {
  props = _props;
  build();
}

TextBannerProps& TextBanner::getProps() { return props; }

const TextBannerProps& TextBanner::getProps() const { return props; }

void TextBanner::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void TextBanner::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

std::pair<int, int> TextBanner::measureTextScaled() const {
  if (props.text.empty()) {
    return {0, 0};
  }

  TextLine measureLine(window);
  measureLine.setScale(1.f);
  TextLineProps measureProps;
  measureProps.fontFamily = props.fontFamily;
  measureProps.fontSize = props.fontSize;
  measureProps.fontColor = props.fontColor;
  measureProps.textBlocks.pushBack(TextBlock{.text = props.text});
  measureLine.setProps(measureProps);
  return measureLine.getDims();
}

std::pair<int, int>
TextBanner::calculateBannerScreenPosition(int bannerScaledWidth,
                                          int bannerScaledHeight) const {
  const int containerX = style.x + static_cast<int>(props.location.first * style.scale);
  const int containerY = style.y + static_cast<int>(props.location.second * style.scale);
  const int containerWidth = static_cast<int>(props.dims.first * style.scale);
  const int containerHeight = static_cast<int>(props.dims.second * style.scale);

  int bannerX = containerX;
  int bannerY = containerY;

  switch (props.corner) {
  case TextBannerCorner::LEFT_TOP:
    break;
  case TextBannerCorner::RIGHT_TOP:
    bannerX = containerX + containerWidth - bannerScaledWidth;
    break;
  case TextBannerCorner::LEFT_BOTTOM:
    bannerY = containerY + containerHeight - bannerScaledHeight;
    break;
  case TextBannerCorner::RIGHT_BOTTOM:
    bannerX = containerX + containerWidth - bannerScaledWidth;
    bannerY = containerY + containerHeight - bannerScaledHeight;
    break;
  }

  return {bannerX, bannerY};
}

const std::pair<int, int> TextBanner::getDims() const {
  const auto [textWidth, textHeight] = measureTextScaled();
  const int borderScaled = static_cast<int>(props.outsetBorderSize * style.scale);
  const int paddingScaled = static_cast<int>(props.padding * style.scale);
  const int bannerWidth = textWidth + paddingScaled * 2 + borderScaled * 2;
  const int bannerHeight = textHeight + paddingScaled * 2 + borderScaled * 2;
  return {bannerWidth, bannerHeight};
}

void TextBanner::build() {
  children.clear();

  auto textLine = new TextLine(window, this);
  textLine->setScale(1.f);
  TextLineProps lineProps;
  lineProps.fontFamily = props.fontFamily;
  lineProps.fontSize = props.fontSize;
  lineProps.fontColor = props.fontColor;
  lineProps.textAlign = TextAlign::LEFT_CENTER;
  lineProps.textBlocks.pushBack(TextBlock{.text = props.text});
  textLine->setProps(lineProps);

  auto [textWidth, textHeight] = textLine->getDims();
  auto paddingScaled = static_cast<int>(props.padding * style.scale);
  int textX = 0;
  int textY = 0;
  switch (props.corner) {
  case TextBannerCorner::LEFT_TOP:
    textX = props.location.first + paddingScaled;
    textY = props.location.second + (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::RIGHT_TOP:
    textX = props.location.first + props.dims.first - textWidth - paddingScaled;
    textY = props.location.second + (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::LEFT_BOTTOM:
    textX = props.location.first + paddingScaled;
    textY =
        props.location.second + props.dims.second - (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::RIGHT_BOTTOM:
    textX = props.location.first + props.dims.first - textWidth - paddingScaled;
    textY =
        props.location.second + props.dims.second - (paddingScaled * 2 + textHeight) / 2;
    break;
  }
  textLine->setPos(textX, textY);

  auto background = new OutsetRectangle(window, this);
  background->setPos(textX - paddingScaled,
                     textY - (paddingScaled * 2 + textHeight) / 2);
  background->setScale(1.f);
  background->setProps(OutsetRectangleProps{
      .width = textWidth + paddingScaled * 2,
      .height = textHeight + paddingScaled * 2,
      .color = props.backgroundColor,
      .borderSize = props.outsetBorderSize,
  });

  addChild(background);
  addChild(textLine);

  auto [bw, bh] = getDims();
  style.width = style.scale > 0.f ? static_cast<int>(bw / style.scale) : bw;
  style.height = style.scale > 0.f ? static_cast<int>(bh / style.scale) : bh;
}

void TextBanner::render(int dt) { UiElement::render(dt); }

} // namespace ui
