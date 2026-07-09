#include "TextParagraph.h"
#include "state/StateManager.h"
#include "Quad.h"
#include "sdl2w/Draw.h"
#include "ui/FontScale.h"
#include <algorithm>
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"

namespace ui {

namespace {

std::pair<int, int> measureLine(sdl2w::Draw& draw,
                                const bmin::String& lineText,
                                const sdl2w::RenderTextParams& params) {
  const bmin::String& sample = lineText.empty() ? bmin::String(" ") : lineText;
  return draw.measureText(bmin::toStringView(sample), params);
}

} // namespace

TextParagraph::TextParagraph(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  quad = bmin::makeUnique<Quad>(window, this);
  quad->setId("textParagraphQuad");
}

void TextParagraph::setProps(const TextParagraphProps& _props) {
  props = _props;
  build();
}

TextParagraphProps& TextParagraph::getProps() { return props; }

const TextParagraphProps& TextParagraph::getProps() const { return props; }

void TextParagraph::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void TextParagraph::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

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

int TextParagraph::getContentHeight() const {
  if (generatedBlocks.empty() || lineHeightFromFont <= 0) {
    return 0;
  }
  return static_cast<int>(getNumLines()) * (props.lineSpacing + lineHeightFromFont);
}

const std::pair<int, int> TextParagraph::getDims() const {
  const int contentHeight = getContentHeight();
  const int logicalW = style.width + 2 * props.padding;
  const int logicalH = contentHeight + 2 * props.padding;
  return {static_cast<int>(logicalW * style.scale),
          static_cast<int>(logicalH * style.scale)};
}

void TextParagraph::build() {
  generatedBlocks.clear();
  style.width = props.width;
  auto& draw = window->getDraw();
  int fontScale = 0;
  try {
    auto stateManager = getStateManager();
    if (!stateManager) {
      throw std::runtime_error("StateManager not set");
    }
    fontScale = stateManager->getState().settings.fontScale;
  } catch (...) {
    // Some isolated UI tests do not initialize a StateManager.
    fontScale = 0;
  }

  int lineNumber = 0;
  bmin::String lineAggregate;
  bmin::String nextWord;

  auto flushCurrentLine = [&](const TextBlock& block,
                              const sdl2w::RenderTextParams& params) {
    auto [textWidth, textHeight] = measureLine(draw, lineAggregate, params);
    lineHeightFromFont = std::max(lineHeightFromFont, textHeight);
    generatedBlocks.pushBack(TextParagraphGeneratedBlock{
        lineNumber,
        block,
        lineAggregate,
        lineAggregate.empty() ? 0 : textWidth,
        textHeight});
    lineNumber++;
    lineAggregate.clear();
  };

  auto appendToCurrentLine = [&](const bmin::String& text,
                                   const TextBlock& block,
                                   const sdl2w::RenderTextParams& params) {
    if (text.empty()) {
      return;
    }
    const bmin::String candidate = lineAggregate + text;
    auto [candidateWidth, textHeight] = measureLine(draw, candidate, params);
    lineHeightFromFont = std::max(lineHeightFromFont, textHeight);

    if (!lineAggregate.empty() && candidateWidth >= style.width) {
      flushCurrentLine(block, params);
      lineAggregate = text;
      auto [wordWidth, wordHeight] = measureLine(draw, lineAggregate, params);
      lineHeightFromFont = std::max(lineHeightFromFont, wordHeight);
      if (wordWidth >= style.width) {
        flushCurrentLine(block, params);
      }
    } else {
      lineAggregate = candidate;
      if (lineAggregate.size() == text.size() && candidateWidth >= style.width) {
        flushCurrentLine(block, params);
      }
    }
  };

  const TextBlock* lastProcessedBlock = nullptr;

  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }
    lastProcessedBlock = &block;

    auto fontFamily = block.fontFamily.value_or(props.fontFamily);
    auto fontSize = block.fontSize.value_or(props.fontSize);
    auto fontColor = block.fontColor.value_or(props.fontColor);
    auto fontName = TextLine::getFontNameFromFamily(fontFamily);

    sdl2w::RenderTextParams params;
    params.fontName = fontName.cStr();
    params.fontSize = ui::applyFontScale(fontSize, fontScale);
    params.color = fontColor;
    params.centered = false;

    lineHeightFromFont = 0;

    for (size_t i = 0; i < block.text.size(); i++) {
      auto c = block.text[i];
      if (c == '\r') {
        continue;
      }
      if (i > 0 && block.text[i - 1] == ' ' && c == ' ') {
        // skip multiple spaces in a row
        continue;
      }
      auto isLastLetter = i + 1 == block.text.size();
      if (c == '\n') {
        if (!nextWord.empty()) {
          appendToCurrentLine(nextWord, block, params);
          nextWord.clear();
        }
        flushCurrentLine(block, params);
      } else if (c == ' ') {
        appendToCurrentLine(nextWord + ' ', block, params);
        nextWord.clear();
      } else if (isLastLetter) {
        appendToCurrentLine(nextWord + c, block, params);
        nextWord.clear();
      } else {
        nextWord += c;
      }
    }

    if (!nextWord.empty()) {
      appendToCurrentLine(nextWord, block, params);
      nextWord.clear();
    }
  }

  if (!lineAggregate.empty() && lastProcessedBlock != nullptr) {
    auto fontFamily = lastProcessedBlock->fontFamily.value_or(props.fontFamily);
    auto fontSize = lastProcessedBlock->fontSize.value_or(props.fontSize);
    auto fontColor = lastProcessedBlock->fontColor.value_or(props.fontColor);
    sdl2w::RenderTextParams params;
    params.fontName = TextLine::getFontNameFromFamily(fontFamily);
    params.fontSize = ui::applyFontScale(fontSize, fontScale);
    params.color = fontColor;
    params.centered = false;
    flushCurrentLine(*lastProcessedBlock, params);
  }

  while (!quad->getChildren().empty()) {
    quad->removeChildAtIndex(0);
  }

  // Create TextLine children inside the quad (texture-local coordinates)
  if (!generatedBlocks.empty()) {
    auto currentLineNumber = -1;
    bmin::DynArray<TextBlock> currentLineBlocks;
    auto currentY = props.padding;
    int currentLineMaxHeight = 0;

    for (const auto& genBlock : generatedBlocks) {
      if (genBlock.lineNumber != currentLineNumber) {
        if (!currentLineBlocks.empty()) {
          auto textLine = new TextLine(window, quad.get());
          textLine->setPos(props.padding, currentY);
          textLine->setScale(1.f);

          TextLineProps lineProps;
          lineProps.textBlocks = currentLineBlocks;
          lineProps.fontFamily =
              genBlock.textBlock.fontFamily.value_or(props.fontFamily);
          lineProps.fontSize = genBlock.textBlock.fontSize.value_or(props.fontSize);
          lineProps.fontColor = genBlock.textBlock.fontColor.value_or(props.fontColor);
          lineProps.textAlign = props.textAlign;
          textLine->setProps(lineProps);

          quad->addChild(textLine);
          currentLineBlocks.clear();

          currentY += currentLineMaxHeight + props.lineSpacing;
          currentLineMaxHeight = 0;
        }
        currentLineNumber = genBlock.lineNumber;
      }

      currentLineMaxHeight = std::max(currentLineMaxHeight, genBlock.textHeight);

      TextBlock textBlock;
      textBlock.text = genBlock.text;
      textBlock.fontFamily = genBlock.textBlock.fontFamily.value_or(props.fontFamily);
      textBlock.fontSize = genBlock.textBlock.fontSize.value_or(props.fontSize);
      textBlock.fontColor = genBlock.textBlock.fontColor.value_or(props.fontColor);
      currentLineBlocks.pushBack(textBlock);
    }

    if (!currentLineBlocks.empty()) {
      auto textLine = new TextLine(window, quad.get());
      const auto& lastGenBlock = generatedBlocks.back();

      textLine->setPos(props.padding, currentY);
      textLine->setScale(1.f);

      TextLineProps lineProps;
      lineProps.textBlocks = currentLineBlocks;
      lineProps.fontFamily =
          lastGenBlock.textBlock.fontFamily.value_or(props.fontFamily);
      lineProps.fontSize = lastGenBlock.textBlock.fontSize.value_or(props.fontSize);
      lineProps.fontColor = lastGenBlock.textBlock.fontColor.value_or(props.fontColor);
      lineProps.textAlign = props.textAlign;
      textLine->setProps(lineProps);

      quad->addChild(textLine);
    }
  }

  const int contentHeight = getContentHeight();
  style.height = contentHeight + 2 * props.padding;

  quad->setPos(style.x, style.y);
  quad->setScale(style.scale);
  QuadProps quadProps;
  quadProps.width = style.width + 2 * props.padding;
  quadProps.height = contentHeight + 2 * props.padding;
  quadProps.bgColor = props.bgColor;
  quad->setProps(quadProps);
}

void TextParagraph::render(int dt) {
  if (quad) {
    quad->render(dt);
  }
}

} // namespace ui
