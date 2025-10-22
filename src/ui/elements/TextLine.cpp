#include "TextLine.h"
#include "lib/sdl2w/Draw.h"
#include "ui/UiElement.h"

namespace ui {

TextLine::TextLine(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

std::string TextLine::getFontNameFromFamily(FontFamily fontFamily) {
  auto fontName = std::string("monofonto");
  switch (fontFamily) {
  case FontFamily::PARAGRAPH:
    fontName = "monofonto";
    break;
  case FontFamily::H1:
    fontName = "monofonto";
    break;
  case FontFamily::H2:
    fontName = "monofonto";
    break;
  case FontFamily::H3:
    fontName = "monofonto";
    break;
  }
  return fontName;
}

void TextLine::setProps(const TextLineProps& _props) {
  props = _props;
  build();
}

TextLineProps& TextLine::getProps() { return props; }

const TextLineProps& TextLine::getProps() const { return props; }

void TextLine::build() {
  textRenderables.clear();

  // Starting position for text rendering
  auto currentX = style.x;
  auto currentY = style.y;

  int totalWidth = 0;
  int totalHeight = 0;

  // Render each text block
  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    auto& draw = window->getDraw();

    // Determine which styles to use (block overrides or base style)
    auto fontFamily = block.fontFamily.value_or(style.fontFamily);
    auto fontSize = block.fontSize.value_or(style.fontSize);
    auto fontColor = block.fontColor.value_or(style.fontColor);
    auto fontName = getFontNameFromFamily(fontFamily);

    // Set up text rendering parameters
    auto tlParams = std::make_unique<TextLineRenderTextParams>();
    tlParams->text = block.text;
    auto& renderTextParams = tlParams->params;
    renderTextParams.fontName = fontName;
    renderTextParams.fontSize = fontSize;
    renderTextParams.x = currentX;
    renderTextParams.y = currentY;
    renderTextParams.color = fontColor;
    renderTextParams.centered = style.textAlign == TextAlign::CENTER;
    auto [textWidth, textHeight] = draw.measureText(block.text, renderTextParams);
    if (style.textAlign == TextAlign::LEFT_CENTER) {
      renderTextParams.y -= textHeight / 2;
    } else if (style.textAlign == TextAlign::LEFT_BOTTOM) {
      renderTextParams.y -= textHeight;
    }
    textRenderables.push_back(std::move(tlParams));

    totalHeight = std::max(totalHeight, textHeight);
    totalWidth += textWidth;
    currentX += textWidth;
  }

  style.width = totalWidth;
  style.height = totalHeight;
}

void TextLine::render() {
  if (props.textBlocks.empty()) {
    return;
  }

  auto& draw = window->getDraw();

  for (const auto& rtParams : textRenderables) {
    draw.drawText(rtParams->text, rtParams->params);
  }
}

} // namespace ui
