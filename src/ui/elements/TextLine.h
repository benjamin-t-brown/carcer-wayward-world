#pragma once

#include "../UiElement.h"
#include <SDL2/SDL_pixels.h>
#include <optional>
#include <string>
#include <vector>

namespace ui {

// Individual text block with optional style overrides
struct TextBlock {
  std::string text;

  // Optional style overrides - if not set, uses base style from UiElement
  std::optional<FontFamily> fontFamily;
  std::optional<sdl2w::TextSize> fontSize;
  std::optional<SDL_Color> fontColor;
};

// TextLine-specific properties
struct TextLineProps {
  std::vector<TextBlock> textBlocks;
};

struct TextLineRenderTextParams {
  std::string text;
  sdl2w::RenderTextParams params;
};

// TextLine element - renders a stylized line of text
// Uses Position and TextParams from BaseStyle
class TextLine : public UiElement {
private:
  TextLineProps props;
  std::vector<std::unique_ptr<TextLineRenderTextParams>> textRenderables;

public:
  TextLine(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextLine() override = default;

  // Static utility method to convert FontFamily to font name
  static std::string getFontNameFromFamily(FontFamily fontFamily);

  // Setters and getters for text line properties
  void setProps(const TextLineProps& _props);
  TextLineProps& getProps();
  const TextLineProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
