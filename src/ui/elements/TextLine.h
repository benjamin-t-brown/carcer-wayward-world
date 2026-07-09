#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/TextStyle.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"

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
