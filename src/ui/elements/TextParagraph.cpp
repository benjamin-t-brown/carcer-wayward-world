#include "TextParagraph.h"
#include "lib/sdl2w/Draw.h"
#include <sstream>

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

void TextParagraph::build() {
  generatedBlocks.clear();
  auto& draw = window->getDraw();

  int lineNumber = 0;
  std::stringstream lineAggregate;
  std::stringstream potentialLineAggregate;
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

    for (size_t i = 0; i < block.text.size(); i++) {
      char c = block.text[i];
      if (i > 0 && block.text[i - 1] == ' ') {
        // skip multiple spaces in a row
        continue;
      }
      if (c == ' ' || i + 1 == block.text.size()) {
        auto [textWidth, textHeight] = draw.measureText(
            lineAggregate.str() + " " + potentialLineAggregate.str(), params);
        if (textWidth <= style.width) {
          lineAggregate << potentialLineAggregate.str() << ' ';
          potentialLineAggregate.clear();
        } else {
          size_t len = lineAggregate.str().size();
          if (len == 0) {
            generatedBlocks.push_back( //
                TextParagraphGeneratedBlock{
                    lineNumber, params, potentialLineAggregate.str()});
            lineNumber++;
            lineAggregate.clear();
          } else {
            generatedBlocks.push_back( //
                TextParagraphGeneratedBlock{lineNumber, params, lineAggregate.str()});
            lineNumber++;
            lineAggregate.clear();
            lineAggregate << potentialLineAggregate.str();
          }
          potentialLineAggregate.clear();
        }
      } else if (c == '\n') {
        auto [textWidth, textHeight] = draw.measureText(
            lineAggregate.str() + " " + potentialLineAggregate.str(), params);
        if (textWidth <= style.width) {
          lineAggregate << potentialLineAggregate.str() << ' ';
          potentialLineAggregate.clear();
        }
        generatedBlocks.push_back( //
            TextParagraphGeneratedBlock{lineNumber, params, lineAggregate.str()});
        lineNumber++;
        lineAggregate.clear();
      } else {
        potentialLineAggregate << c;
      }
    }

    if (lineAggregate.str().size() > 0) {
      generatedBlocks.push_back( //
          TextParagraphGeneratedBlock{lineNumber, params, lineAggregate.str()});
    }

    // auto [textWidth, textHeight] = draw.measureText(block.text, params);
    // if (textWidth <= style.width) {
    // }
  }
}

void TextParagraph::render() {
  if (props.textBlocks.empty()) {
    return;
  }

  auto& draw = window->getDraw();

  // Starting position for text rendering
  auto currentX = style.x;
  auto currentY = style.y;
  auto maxWidth = style.width;
  auto maxHeight = style.height;

  // Track the line height for wrapping
  auto lineStartX = style.x;

  // Render each text block with wrapping
  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    // Determine which styles to use (block overrides or base style)
    auto fontFamily = block.fontFamily.value_or(style.fontFamily);
    auto fontSize = block.fontSize.value_or(style.fontSize);
    auto fontColor = block.fontColor.value_or(style.fontColor);

    // Convert FontFamily enum to font name string
    auto fontName = TextLine::getFontNameFromFamily(fontFamily);

    // Set up text rendering parameters
    sdl2w::RenderTextParams params;
    params.fontName = fontName;
    params.fontSize = fontSize;
    params.color = fontColor;
    params.centered = false;

    // Split text into words for wrapping
    auto text = block.text;
    auto wordStart = size_t{0};
    auto pos = size_t{0};

    while (pos <= text.length()) {
      // Find the next word boundary (space or end of string)
      if (pos == text.length() || text[pos] == ' ' || text[pos] == '\n') {
        auto word = text.substr(wordStart, pos - wordStart);

        // Handle explicit newlines
        if (pos < text.length() && text[pos] == '\n') {
          // Render current word if any
          if (!word.empty()) {
            params.x = currentX;
            params.y = currentY;
            auto [wordWidth, wordHeight] = draw.measureText(word, params);
            draw.drawText(word, params);
            currentX += wordWidth;
          }

          // Move to next line
          currentY += static_cast<int>(fontSize);
          currentX = lineStartX;
          wordStart = pos + 1;
          pos++;
          continue;
        }

        if (!word.empty()) {
          // Measure the word
          params.x = currentX;
          params.y = currentY;
          auto [wordWidth, wordHeight] = draw.measureText(word, params);

          // Check if we need to wrap
          if (maxWidth > 0 && currentX + wordWidth > style.x + maxWidth &&
              currentX > lineStartX) {
            // Wrap to next line
            currentY += static_cast<int>(fontSize);
            currentX = lineStartX;
            params.x = currentX;
            params.y = currentY;

            // Check if we've exceeded max height
            if (maxHeight > 0 && currentY + wordHeight > style.y + maxHeight) {
              return; // Stop rendering if we've run out of space
            }
          }

          // Render the word
          draw.drawText(word, params);
          currentX += wordWidth;

          // Add space after word if not at end
          if (pos < text.length() && text[pos] == ' ') {
            params.x = currentX;
            params.y = currentY;
            auto [spaceWidth, spaceHeight] = draw.measureText(" ", params);
            currentX += spaceWidth;
          }
        }

        wordStart = pos + 1;
      }
      pos++;
    }
  }
}

} // namespace ui
