#pragma once

#include "../UiElement.h"
#include "Quad.h"
#include "TextLine.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/colors.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"

namespace ui {

// TextParagraph-specific properties
struct TextParagraphProps {
  bmin::DynArray<TextBlock> textBlocks;
  SDL_Color bgColor = Colors::Transparent;
  int padding = 0;
};

struct TextParagraphGeneratedBlock {
  int lineNumber;
  TextBlock textBlock;
  bmin::String text;
  int textWidth;
  int textHeight;
};

// TextParagraph element - lays out wrapped text into TextLines rendered via an internal Quad
class TextParagraph : public UiElement {
private:
  TextParagraphProps props;
  bmin::DynArray<TextParagraphGeneratedBlock> generatedBlocks;
  bmin::UniquePtr<Quad> quad;

  int lineHeightFromFont = 0;

  int getContentHeight() const;

public:
  TextParagraph(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextParagraph() override = default;

  void setProps(const TextParagraphProps& _props);
  TextParagraphProps& getProps();
  const TextParagraphProps& getProps() const;
  size_t getNumLines() const;
  virtual const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
