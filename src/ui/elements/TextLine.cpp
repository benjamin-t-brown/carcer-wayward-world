#include "TextLine.h"
#include "lib/sdl2w/Draw.h"
#include "ui/UiElement.h"

namespace ui {

TextLine::TextLine(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

std::string TextLine::getFontNameFromFamily(FontFamily fontFamily) {
  auto fontName = std::string("default");
  switch (fontFamily) {
  case FontFamily::PARAGRAPH:
    fontName = "text";
    break;
  case FontFamily::H1:
    fontName = "title";
    break;
  case FontFamily::H2:
    fontName = "title";
    break;
  case FontFamily::H3:
    fontName = "title";
    break;
  }
  return fontName;
}

sdl2w::RenderTextParams TextLine::makeRenderTextParams(const TextBlock& block) const {
  auto fontFamily = block.fontFamily.value_or(style.fontFamily);
  auto fontSize = block.fontSize.value_or(style.fontSize);
  auto fontColor = block.fontColor.value_or(style.fontColor);

  sdl2w::RenderTextParams params;
  params.fontName = getFontNameFromFamily(fontFamily);
  params.fontSize = fontSize;
  params.color = fontColor;
  params.centered = style.textAlign == TextAlign::CENTER;
  return params;
}

void TextLine::setProps(const TextLineProps& _props) {
  props = _props;
  build();
}

TextLineProps& TextLine::getProps() { return props; }

const TextLineProps& TextLine::getProps() const { return props; }

std::pair<int, int> TextLine::calculateTextDims() const {
  if (props.textBlocks.empty()) {
    return {0, 0};
  }

  auto& draw = window->getDraw();
  int totalWidth = 0;
  int totalHeight = 0;

  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    auto [textWidth, textHeight] = draw.measureText(block.text, makeRenderTextParams(block));
    totalWidth += textWidth;
    totalHeight = std::max(totalHeight, textHeight);
  }

  return {totalWidth, totalHeight};
}

const std::pair<int, int> TextLine::getDims() const {
  auto [width, height] = calculateTextDims();
  return {static_cast<int>(width * style.scale),
          static_cast<int>(height * style.scale)};
}

void TextLine::build() {
  textRenderables.clear();

  auto [totalWidth, totalHeight] = calculateTextDims();
  style.width = totalWidth;
  style.height = totalHeight;

  auto currentX = static_cast<int>(style.x * style.scale);
  auto currentY = static_cast<int>(style.y * style.scale);

  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    auto renderTextParams = makeRenderTextParams(block);
    auto [textWidth, textHeight] =
        window->getDraw().measureText(block.text, renderTextParams);

    auto tlParams = std::make_unique<TextLineRenderTextParams>();
    tlParams->text = block.text;
    renderTextParams.x = currentX;
    renderTextParams.y = currentY;
    if (style.textAlign == TextAlign::LEFT_CENTER) {
      renderTextParams.y -= static_cast<int>((textHeight / 2.0) * style.scale);
    } else if (style.textAlign == TextAlign::LEFT_BOTTOM) {
      renderTextParams.y -= static_cast<int>(textHeight * style.scale);
    }
    tlParams->params = renderTextParams;
    textRenderables.push_back(std::move(tlParams));

    currentX += static_cast<int>(textWidth * style.scale);
  }
}

void TextLine::render(int dt) {
  if (props.textBlocks.empty()) {
    return;
  }

  auto& draw = window->getDraw();

  for (const auto& rtParams : textRenderables) {
    draw.drawText(rtParams->text, rtParams->params);
  }
}

} // namespace ui
