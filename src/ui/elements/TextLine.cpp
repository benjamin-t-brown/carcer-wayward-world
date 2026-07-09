#include "TextLine.h"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include "lib/sdl2w/Draw.h"
#include "state/StateManager.h"
#include "ui/FontScale.h"
#include "ui/UiElement.h"

namespace ui {

TextLine::TextLine(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

bmin::String TextLine::getFontNameFromFamily(FontFamily fontFamily) {
  auto fontName = bmin::String("default");
  switch (fontFamily) {
  case FontFamily::TEXT:
    fontName = "text";
    break;
  case FontFamily::TEXT_BOLD:
    fontName = "text-bold";
    break;
  case FontFamily::DEFAULT:
    fontName = "default";
    break;
  case FontFamily::TITLE:
    fontName = "title";
    break;
  }
  return fontName;
}

sdl2w::RenderTextParams TextLine::makeRenderTextParams(const TextBlock& block) const {
  auto fontFamily = block.fontFamily.value_or(style.fontFamily);
  auto baseFontSize = block.fontSize.value_or(style.fontSize);
  auto fontColor = block.fontColor.value_or(style.fontColor);
  int fontScale = 0;
  try {
    auto stateManager = getStateManager();
    if (!stateManager) {
      throw std::runtime_error("StateManager not set");
    }
    fontScale = getStateManager()->getState().settings.fontScale;
  } catch (...) {
    // Some isolated UI tests do not initialize a StateManager.
    fontScale = 0;
  }

  sdl2w::RenderTextParams params;
  params.fontName = getFontNameFromFamily(fontFamily);
  params.fontSize = ui::applyFontScale(baseFontSize, fontScale);
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
    const bmin::String& measureStr = block.text.empty() ? bmin::String(" ") : block.text;
    auto [textWidth, textHeight] =
        draw.measureText(bmin::toStringView(measureStr), makeRenderTextParams(block));
    if (!block.text.empty()) {
      totalWidth += textWidth;
    }
    totalHeight = std::max(totalHeight, textHeight);
  }

  return {totalWidth, totalHeight};
}

const std::pair<int, int> TextLine::getDims() const {
  auto [width, height] = calculateTextDims();
  return {static_cast<int>(width * style.scale), static_cast<int>(height * style.scale)};
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
      // Blank line: reserve vertical space via calculateTextDims(), nothing to draw.
      continue;
    }

    auto renderTextParams = makeRenderTextParams(block);
    auto [textWidth, textHeight] =
        window->getDraw().measureText(bmin::toStringView(block.text), renderTextParams);
    textHeight += 2; // HACK: Measure text doesn't seem accurate per height, so this will
                     // need overrides...

    auto tlParams = bmin::makeUnique<TextLineRenderTextParams>();
    tlParams->text = block.text;
    renderTextParams.x = currentX;
    renderTextParams.y = currentY;
    if (style.textAlign == TextAlign::LEFT_CENTER) {
      renderTextParams.y -= static_cast<int>((textHeight / 2.0) * style.scale);
    } else if (style.textAlign == TextAlign::LEFT_BOTTOM) {
      renderTextParams.y -= static_cast<int>(textHeight * style.scale);
    }
    tlParams->params = renderTextParams;
    textRenderables.pushBack(std::move(tlParams));

    currentX += static_cast<int>(textWidth * style.scale);
  }
}

void TextLine::render(int dt) {
  if (props.textBlocks.empty()) {
    return;
  }

  auto& draw = window->getDraw();

  for (const auto& rtParams : textRenderables) {
    draw.drawText(bmin::toStringView(rtParams->text), rtParams->params);
  }
}

} // namespace ui
