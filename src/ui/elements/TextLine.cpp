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
  // Build logic can be extended here if needed
  // Called when props or style changes
}

void TextLine::render() {
  if (props.textBlocks.empty()) {
    return;
  }

  auto& draw = window->getDraw();

  // Starting position for text rendering
  auto currentX = style.x;
  auto currentY = style.y;

  // Render each text block
  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    // Determine which styles to use (block overrides or base style)
    auto fontFamily = block.fontFamily.value_or(style.fontFamily);
    auto fontSize = block.fontSize.value_or(style.fontSize);
    auto fontColor = block.fontColor.value_or(style.fontColor);

    // Convert FontFamily enum to font name string
    auto fontName = getFontNameFromFamily(fontFamily);

    // Set up text rendering parameters
    sdl2w::RenderTextParams params;
    params.fontName = fontName;
    params.fontSize = fontSize;
    params.x = currentX;
    params.y = currentY;
    params.color = fontColor;
    params.centered = style.textAlign == TextAlign::CENTER;
    auto [textWidth, textHeight] = draw.measureText(block.text, params);
    if (style.textAlign == TextAlign::LEFT_CENTER) {
      params.y -= textHeight / 2;
    } else if (style.textAlign == TextAlign::LEFT_BOTTOM) {
      params.y -= textHeight;
    }

    // Render the text block
    draw.drawText(block.text, params);

    // Move currentX to the right for the next block
    currentX += textWidth;
  }
}

} // namespace ui

