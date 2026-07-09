#include "TextBanner.h"
#include "OutsetRectangle.h"
#include "TextLine.h"

namespace ui {

TextBanner::TextBanner(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  setBaseFontConfig(style, BaseFontConfig::MODAL_TEXT_BOLD);
}

void TextBanner::setProps(const TextBannerProps& _props) {
  props = _props;
  build();
}

TextBannerProps& TextBanner::getProps() { return props; }

const TextBannerProps& TextBanner::getProps() const { return props; }

std::pair<int, int> TextBanner::measureTextScaled() const {
  if (props.text.empty()) {
    return {0, 0};
  }

  TextLine measureLine(window);
  auto& measureStyle = measureLine.getStyle();
  measureStyle.fontFamily = style.fontFamily;
  measureStyle.fontSize = style.fontSize;
  measureStyle.fontColor = style.fontColor;
  measureStyle.scale = 1.f;
  TextLineProps measureProps;
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
  auto& textStyle = textLine->getStyle();
  textStyle.scale = 1.f;
  setBaseFontConfig(textStyle, BaseFontConfig::MODAL_TEXT_BOLD);
  textStyle.textAlign = TextAlign::LEFT_CENTER;
  TextLineProps lineProps;
  lineProps.textBlocks.pushBack(TextBlock{.text = props.text});
  textLine->setProps(lineProps);

  auto [textWidth, textHeight] = textLine->getDims();
  auto paddingScaled = static_cast<int>(props.padding * style.scale);
  switch (props.corner) {
  case TextBannerCorner::LEFT_TOP:
    textStyle.x = props.location.first + paddingScaled;
    textStyle.y = props.location.second + (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::RIGHT_TOP:
    textStyle.x = props.location.first + props.dims.first - textWidth - paddingScaled;
    textStyle.y = props.location.second + (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::LEFT_BOTTOM:
    textStyle.x = props.location.first + paddingScaled;
    textStyle.y =
        props.location.second + props.dims.second - (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::RIGHT_BOTTOM:
    textStyle.x = props.location.first + props.dims.first - textWidth - paddingScaled;
    textStyle.y =
        props.location.second + props.dims.second - (paddingScaled * 2 + textHeight) / 2;
    break;
  }
  textLine->build();

  auto background = new OutsetRectangle(window, this);
  auto& backgroundStyle = background->getStyle();
  backgroundStyle.x = textStyle.x - paddingScaled;
  backgroundStyle.y = textStyle.y - (paddingScaled * 2 + textHeight) / 2;
  backgroundStyle.width = textWidth + paddingScaled * 2;
  backgroundStyle.height = textHeight + paddingScaled * 2;
  backgroundStyle.scale = 1.f;
  background->setProps(OutsetRectangleProps{
      .color = props.backgroundColor,
      // .colorTopRight = Colors::White,
      // .colorBottomLeft = Colors::White,
      .borderSize = props.outsetBorderSize,
  });

  addChild(background);
  addChild(textLine);
}

void TextBanner::render(int dt) { UiElement::render(dt); }

} // namespace ui
