#include "TextParagraph.h"
#include "state/StateManager.h"
#include "Quad.h"
#include "lib/sdl2w/Draw.h"
#include "ui/FontScale.h"
#include <algorithm>
#include <string>

namespace ui {

namespace {

std::pair<int, int> measureLine(sdl2w::Draw& draw,
                                const std::string& lineText,
                                const sdl2w::RenderTextParams& params) {
  const std::string& sample = lineText.empty() ? std::string(" ") : lineText;
  return draw.measureText(sample, params);
}

} // namespace

TextParagraph::TextParagraph(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  quad = std::make_unique<Quad>(window, this);
  quad->setId("textParagraphQuad");
}

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

int TextParagraph::getContentHeight() const {
  if (generatedBlocks.empty() || lineHeightFromFont <= 0) {
    return 0;
  }
  return static_cast<int>(getNumLines()) * (style.lineSpacing + lineHeightFromFont);
}

const std::pair<int, int> TextParagraph::getDims() const {
  const int contentHeight = getContentHeight();
  return {style.width + 2 * props.padding, contentHeight + 2 * props.padding};
}

void TextParagraph::build() {
  generatedBlocks.clear();
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
  std::string lineAggregate;
  std::string nextWord;

  auto flushCurrentLine = [&](const TextBlock& block,
                              const sdl2w::RenderTextParams& params) {
    auto [textWidth, textHeight] = measureLine(draw, lineAggregate, params);
    lineHeightFromFont = std::max(lineHeightFromFont, textHeight);
    generatedBlocks.push_back(TextParagraphGeneratedBlock{
        lineNumber,
        block,
        lineAggregate,
        lineAggregate.empty() ? 0 : textWidth,
        textHeight});
    lineNumber++;
    lineAggregate.clear();
  };

  auto appendToCurrentLine = [&](const std::string& text,
                                   const TextBlock& block,
                                   const sdl2w::RenderTextParams& params) {
    if (text.empty()) {
      return;
    }
    const std::string candidate = lineAggregate + text;
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

    auto fontFamily = block.fontFamily.value_or(style.fontFamily);
    auto fontSize = block.fontSize.value_or(style.fontSize);
    auto fontColor = block.fontColor.value_or(style.fontColor);
    auto fontName = TextLine::getFontNameFromFamily(fontFamily);

    sdl2w::RenderTextParams params;
    params.fontName = fontName;
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
    auto fontFamily = lastProcessedBlock->fontFamily.value_or(style.fontFamily);
    auto fontSize = lastProcessedBlock->fontSize.value_or(style.fontSize);
    auto fontColor = lastProcessedBlock->fontColor.value_or(style.fontColor);
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
    std::vector<TextBlock> currentLineBlocks;
    auto currentY = props.padding;
    int currentLineMaxHeight = 0;

    for (const auto& genBlock : generatedBlocks) {
      if (genBlock.lineNumber != currentLineNumber) {
        if (!currentLineBlocks.empty()) {
          auto textLine = new TextLine(window, quad.get());

          ui::BaseStyle lineStyle;
          lineStyle.x = props.padding;
          lineStyle.y = currentY;
          lineStyle.width = style.width;
          lineStyle.height = currentLineMaxHeight;
          lineStyle.fontFamily = genBlock.textBlock.fontFamily.value_or(style.fontFamily);
          lineStyle.fontSize = genBlock.textBlock.fontSize.value_or(style.fontSize);
          lineStyle.fontColor = genBlock.textBlock.fontColor.value_or(style.fontColor);
          lineStyle.textAlign = style.textAlign;
          lineStyle.scale = 1.f;
          textLine->setStyle(lineStyle);

          TextLineProps lineProps;
          lineProps.textBlocks = currentLineBlocks;
          textLine->setProps(lineProps);

          quad->addChild(textLine);
          currentLineBlocks.clear();

          currentY += currentLineMaxHeight + style.lineSpacing;
          currentLineMaxHeight = 0;
        }
        currentLineNumber = genBlock.lineNumber;
      }

      currentLineMaxHeight = std::max(currentLineMaxHeight, genBlock.textHeight);

      TextBlock textBlock;
      textBlock.text = genBlock.text;
      textBlock.fontFamily = genBlock.textBlock.fontFamily.value_or(style.fontFamily);
      textBlock.fontSize = genBlock.textBlock.fontSize.value_or(style.fontSize);
      textBlock.fontColor = genBlock.textBlock.fontColor.value_or(style.fontColor);
      currentLineBlocks.push_back(textBlock);
    }

    if (!currentLineBlocks.empty()) {
      auto textLine = new TextLine(window, quad.get());
      const auto& lastGenBlock = generatedBlocks.back();

      ui::BaseStyle lineStyle;
      lineStyle.x = props.padding;
      lineStyle.y = currentY;
      lineStyle.width = style.width;
      lineStyle.height = currentLineMaxHeight;
      lineStyle.fontFamily = lastGenBlock.textBlock.fontFamily.value_or(style.fontFamily);
      lineStyle.fontSize = lastGenBlock.textBlock.fontSize.value_or(style.fontSize);
      lineStyle.fontColor = lastGenBlock.textBlock.fontColor.value_or(style.fontColor);
      lineStyle.textAlign = style.textAlign;
      lineStyle.scale = 1.f;
      textLine->setStyle(lineStyle);

      TextLineProps lineProps;
      lineProps.textBlocks = currentLineBlocks;
      textLine->setProps(lineProps);

      quad->addChild(textLine);
    }
  }

  const int contentHeight = getContentHeight();
  auto& quadStyle = quad->getStyle();
  quadStyle.x = style.x;
  quadStyle.y = style.y;
  quadStyle.width = style.width + 2 * props.padding;
  quadStyle.height = contentHeight + 2 * props.padding;
  quadStyle.scale = style.scale;

  QuadProps quadProps;
  quadProps.bgColor = props.bgColor;
  quad->setProps(quadProps);
  quad->build();
}

void TextParagraph::render(int dt) {
  if (quad) {
    quad->render(dt);
  }
}

} // namespace ui
