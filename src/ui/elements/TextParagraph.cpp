#include "TextParagraph.h"
#include "lib/sdl2w/Draw.h"

namespace ui {

TextParagraph::TextParagraph(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void TextParagraph::setProps(const TextParagraphProps& _props) {
  props = _props;
  build();
}

TextParagraphProps& TextParagraph::getProps() { return props; }

const TextParagraphProps& TextParagraph::getProps() const { return props; }

size_t TextParagraph::getNumLines() const {
  if (generatedBlocks.empty()) {
    return 0;
  }

  int maxLineNumber = 0;
  for (const auto& block : generatedBlocks) {
    if (block.lineNumber > maxLineNumber) {
      maxLineNumber = block.lineNumber;
    }
  }

  return static_cast<size_t>(maxLineNumber + 1);
}

const std::pair<int, int> TextParagraph::getDims() const {
  return {style.width, getNumLines() * (style.lineSpacing + lineHeightFromFont)};
}

void TextParagraph::build() {
  generatedBlocks.clear();
  auto& draw = window->getDraw();

  int lineNumber = 0;
  int aggregateWidth = 0;
  std::string lineAggregate;
  std::string nextWord;
  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    auto fontFamily = block.fontFamily.value_or(style.fontFamily);
    auto fontSize = block.fontSize.value_or(style.fontSize);
    auto fontColor = block.fontColor.value_or(style.fontColor);
    auto fontName = TextLine::getFontNameFromFamily(fontFamily);

    sdl2w::RenderTextParams params;
    params.fontName = fontName;
    params.fontSize = fontSize;
    params.color = fontColor;
    params.centered = false;

    lineHeightFromFont = 0;

    for (size_t i = 0; i < block.text.size(); i++) {
      auto c = block.text[i];
      if (i > 0 && block.text[i - 1] == ' ' && c == ' ') {
        // skip multiple spaces in a row
        continue;
      }
      auto isLastLetter = i + 1 == block.text.size();
      if (c == ' ' || isLastLetter) {
        auto [wordTextWidth, textHeight] = draw.measureText(nextWord, params);
        lineHeightFromFont = std::max(lineHeightFromFont, textHeight);
        auto [wordAndSpaceTextWidth, textHeight2] =
            draw.measureText(nextWord + c, params);
        auto totalPotentialWidth =
            aggregateWidth + (isLastLetter ? wordAndSpaceTextWidth : wordTextWidth);
        if (totalPotentialWidth < style.width) {
          // add word to current line
          lineAggregate += nextWord + c;
          aggregateWidth += wordAndSpaceTextWidth;
          nextWord.clear();
        } else {
          // word overflows, so new line
          size_t len = lineAggregate.size() + aggregateWidth;
          if (len == 0) {
            // if the word overflows but there's nothing on the line, just drop it in the
            // current line anyway and finalize it
            generatedBlocks.push_back( //
                TextParagraphGeneratedBlock{
                    lineNumber, block, nextWord + c, wordAndSpaceTextWidth, textHeight});
            lineNumber++;
            lineAggregate.clear();
            aggregateWidth = 0;
          } else {
            // finalize block for current line, add potential word to next line
            generatedBlocks.push_back( //
                TextParagraphGeneratedBlock{
                    lineNumber, block, lineAggregate, aggregateWidth, textHeight});
            lineNumber++;
            lineAggregate.clear();
            lineAggregate = nextWord + c;
            aggregateWidth = wordAndSpaceTextWidth;
          }
          nextWord.clear();
        }
      } else if (c == '\n') {
        auto [wordTextWidth, textHeight] = draw.measureText(nextWord, params);
        lineHeightFromFont = std::max(lineHeightFromFont, textHeight);
        auto totalPotentialWidth = aggregateWidth + wordTextWidth;
        if (totalPotentialWidth <= style.width) {
          lineAggregate += nextWord;
          nextWord.clear();
        }
        generatedBlocks.push_back( //
            TextParagraphGeneratedBlock{
                lineNumber, block, lineAggregate, totalPotentialWidth, textHeight});
        lineNumber++;
        lineAggregate.clear();
        aggregateWidth = 0;
      } else {
        nextWord += c;
      }
    }

    if (lineAggregate.size() > 0) {
      auto fontFamily = block.fontFamily.value_or(style.fontFamily);
      auto fontSize = block.fontSize.value_or(style.fontSize);
      auto fontColor = block.fontColor.value_or(style.fontColor);
      auto fontName = TextLine::getFontNameFromFamily(fontFamily);

      sdl2w::RenderTextParams params;
      params.fontName = fontName;
      params.fontSize = fontSize;
      params.color = fontColor;
      params.centered = false;
      auto [textWidth, textHeight] = draw.measureText(lineAggregate, params);
      lineHeightFromFont = std::max(lineHeightFromFont, textHeight);
      generatedBlocks.push_back( //
          TextParagraphGeneratedBlock{
              lineNumber, block, lineAggregate, aggregateWidth, textHeight});
      lineAggregate.clear();
    }
  }

  // Clear existing children
  children.clear();

  // Create TextLine children for each generated line
  if (!generatedBlocks.empty()) {
    // Group blocks by line number
    auto currentLineNumber = -1;
    std::vector<TextBlock> currentLineBlocks;
    auto currentY = style.y;

    for (const auto& genBlock : generatedBlocks) {
      if (genBlock.lineNumber != currentLineNumber) {
        // Create TextLine for previous line if it has blocks
        if (!currentLineBlocks.empty()) {
          auto textLine = std::make_unique<TextLine>(window, this);

          // Set position for this line
          ui::BaseStyle lineStyle;
          lineStyle.x = style.x;
          lineStyle.y = currentY;
          lineStyle.width = style.width;
          lineStyle.height = genBlock.textHeight;
          lineStyle.fontFamily = genBlock.textBlock.fontFamily.value_or(style.fontFamily);
          lineStyle.fontSize = genBlock.textBlock.fontSize.value_or(style.fontSize);
          lineStyle.fontColor = genBlock.textBlock.fontColor.value_or(style.fontColor);
          lineStyle.textAlign = style.textAlign;
          textLine->setStyle(lineStyle);

          // Set text blocks
          TextLineProps lineProps;
          lineProps.textBlocks = currentLineBlocks;
          textLine->setProps(lineProps);

          children.push_back(std::move(textLine));
          currentLineBlocks.clear();

          // Update Y position for next line (height + line spacing)
          currentY += genBlock.textHeight + style.lineSpacing;
        }
        currentLineNumber = genBlock.lineNumber;
      }

      // Create TextBlock from generated block
      TextBlock textBlock;
      textBlock.text = genBlock.text;
      textBlock.fontFamily = genBlock.textBlock.fontFamily.value_or(style.fontFamily);
      textBlock.fontSize = genBlock.textBlock.fontSize.value_or(style.fontSize);
      textBlock.fontColor = genBlock.textBlock.fontColor.value_or(style.fontColor);
      currentLineBlocks.push_back(textBlock);
    }

    // Create TextLine for the last line
    if (!currentLineBlocks.empty() && !generatedBlocks.empty()) {
      auto textLine = std::make_unique<TextLine>(window, this);
      const auto& lastGenBlock = generatedBlocks.back();

      ui::BaseStyle lineStyle;
      lineStyle.x = style.x;
      lineStyle.y = currentY;
      lineStyle.width = style.width;
      lineStyle.height = lastGenBlock.textHeight;
      lineStyle.fontFamily = lastGenBlock.textBlock.fontFamily.value_or(style.fontFamily);
      lineStyle.fontSize = lastGenBlock.textBlock.fontSize.value_or(style.fontSize);
      lineStyle.fontColor = lastGenBlock.textBlock.fontColor.value_or(style.fontColor);
      lineStyle.textAlign = style.textAlign;
      textLine->setStyle(lineStyle);

      TextLineProps lineProps;
      lineProps.textBlocks = currentLineBlocks;
      textLine->setProps(lineProps);

      children.push_back(std::move(textLine));
    }
  }
}

void TextParagraph::render(int dt) {
  // Render all TextLine children
  UiElement::render(dt);
}

} // namespace ui
