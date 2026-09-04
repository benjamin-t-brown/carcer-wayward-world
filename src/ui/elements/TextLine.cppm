module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <stdexcept>
#include <algorithm>

export module carcer.ui.elements:TextLine;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.SdlPixels;
export import carcer.ui.TextStyle;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.ui.FontScale;
#include "macros.h"

export {

// --- from ui/elements/TextLine.h ---
// IWYU pragma: keep

namespace ui {

// Individual text block with optional style overrides
struct TextBlock {
  bmin::String text;

  // Optional style overrides - if not set, uses TextLineProps defaults
  std::optional<FontFamily> fontFamily;
  std::optional<sdl2w::TextSize> fontSize;
  std::optional<SDL_Color> fontColor;
};

// TextLine-specific properties
struct TextLineProps {
  bmin::DynArray<TextBlock> textBlocks;
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
  TextAlign textAlign = TextAlign::LEFT_TOP;
};

struct TextLineRenderTextParams {
  bmin::String text;
  sdl2w::RenderTextParams params;
};

// TextLine element - renders a stylized line of text
class TextLine : public UiElement {
private:
  TextLineProps props;
  bmin::DynArray<bmin::UniquePtr<TextLineRenderTextParams>> textRenderables;

  sdl2w::RenderTextParams makeRenderTextParams(const TextBlock& block) const;

public:
  TextLine(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextLine() override = default;

  // Static utility method to convert FontFamily to font name
  static bmin::String getFontNameFromFamily(FontFamily fontFamily);

  void setProps(const TextLineProps& _props);
  TextLineProps& getProps();
  const TextLineProps& getProps() const;

  // Position is baked into renderables during build
  void setPos(int x, int y) override;
  void setScale(float scale) override;

  std::pair<int, int> calculateTextDims() const;
  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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
  auto fontFamily = block.fontFamily.value_or(props.fontFamily);
  auto baseFontSize = block.fontSize.value_or(props.fontSize);
  auto fontColor = block.fontColor.value_or(props.fontColor);
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
  params.centered = props.textAlign == TextAlign::CENTER;
  return params;
}

void TextLine::setProps(const TextLineProps& _props) {
  props = _props;
  build();
}

TextLineProps& TextLine::getProps() { return props; }

const TextLineProps& TextLine::getProps() const { return props; }

void TextLine::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void TextLine::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

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
    if (props.textAlign == TextAlign::LEFT_CENTER) {
      renderTextParams.y -= static_cast<int>((textHeight / 2.0) * style.scale);
    } else if (props.textAlign == TextAlign::LEFT_BOTTOM) {
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
