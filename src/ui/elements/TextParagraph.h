#pragma once

#include "../UiElement.h"
#include "TextLine.h"
#include <vector>

namespace ui {

// TextParagraph-specific properties
struct TextParagraphProps {
  std::vector<TextBlock> textBlocks;
};

struct TextParagraphGeneratedBlock {
  int lineNumber;
  // sdl2w::RenderTextParams params;
  TextBlock textBlock;
  std::string text;
  int textWidth;
  int textHeight;
};

// TextParagraph element - renders a block of wrapped text
// Uses Position, Size, and TextParams from BaseStyle
class TextParagraph : public UiElement {
private:
  TextParagraphProps props;
  std::vector<TextParagraphGeneratedBlock> generatedBlocks;

public:
  TextParagraph(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextParagraph() override = default;

  // Setters and getters for text paragraph properties
  void setProps(const TextParagraphProps& _props);
  TextParagraphProps& getProps();
  const TextParagraphProps& getProps() const;
  size_t getNumLines() const;

  void build() override;
  void render() override;
};

} // namespace ui

