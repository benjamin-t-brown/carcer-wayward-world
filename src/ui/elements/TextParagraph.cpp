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

// Blank lines already store their gap height; content lines scale only the advance
// between lines so glyphs can still paint at full measured height.
int lineBoxAdvance(int glyphOrBlankHeight, bool isBlank, float lineHeightScale) {
  if (isBlank) {
    return glyphOrBlankHeight;
  }
  return std::max(1, static_cast<int>(glyphOrBlankHeight * lineHeightScale));
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
  if (generatedBlocks.empty()) {
    return 0;
  }

  // Line boxes may be shorter than glyphs (lineHeightScale < 1), so height is the
  // max bottom edge of full glyph bounds, not the sum of scaled line boxes alone.
  int y = 0;
  int maxBottom = 0;
  int currentLine = -1;
  int lineMaxGlyphH = 0;
  bool lineIsBlank = true;
  int linesCounted = 0;

  auto finishLine = [&]() {
    if (currentLine < 0) {
      return;
    }
    if (linesCounted > 0) {
      y += props.lineSpacing;
    }
    maxBottom = std::max(maxBottom, y + lineMaxGlyphH);
    y += lineBoxAdvance(lineMaxGlyphH, lineIsBlank, props.lineHeightScale);
    linesCounted++;
  };

  for (const auto& block : generatedBlocks) {
    if (block.lineNumber != currentLine) {
      finishLine();
      currentLine = block.lineNumber;
      lineMaxGlyphH = block.textHeight;
      lineIsBlank = block.text.empty();
    } else {
      lineMaxGlyphH = std::max(lineMaxGlyphH, block.textHeight);
      if (!block.text.empty()) {
        lineIsBlank = false;
      }
    }
  }
  finishLine();
  return maxBottom;
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
  int currentLineWidth = 0;
  bmin::String segmentText;
  bmin::String nextWord;

  // Emit the current TextBlock's open segment. endLine advances to the next
  // visual line; emitEmptyIfNoSegment keeps blank-line (`\n`) behavior.
  auto emitSegment = [&](const TextBlock& block,
                         const sdl2w::RenderTextParams& params,
                         bool endLine,
                         bool emitEmptyIfNoSegment) {
    if (!segmentText.empty()) {
      auto [textWidth, textHeight] = measureLine(draw, segmentText, params);
      // Store full glyph height; lineHeightScale is applied when advancing lines.
      generatedBlocks.pushBack(TextParagraphGeneratedBlock{
          lineNumber,
          block,
          segmentText,
          textWidth,
          textHeight});
      segmentText.clear();
    } else if (emitEmptyIfNoSegment && currentLineWidth == 0) {
      // Blank line from a double line-break (`\n\n`): scaled paragraph gap.
      auto [textWidth, textHeight] = measureLine(draw, segmentText, params);
      (void)textWidth;
      const int blankLineHeight = std::max(
          0, static_cast<int>(textHeight * props.blankLineHeightScale));
      generatedBlocks.pushBack(TextParagraphGeneratedBlock{
          lineNumber, block, segmentText, 0, blankLineHeight});
    }

    if (endLine) {
      lineNumber++;
      currentLineWidth = 0;
    }
  };

  auto appendToCurrentLine = [&](const bmin::String& text,
                                 const TextBlock& block,
                                 const sdl2w::RenderTextParams& params) {
    if (text.empty()) {
      return;
    }
    auto [pieceWidth, pieceHeight] = measureLine(draw, text, params);
    (void)pieceHeight;

    if (currentLineWidth > 0 && currentLineWidth + pieceWidth >= style.width) {
      // Wrap: close this visual line (keep any prior same-line segments).
      emitSegment(block, params, true, false);
      segmentText = text;
      currentLineWidth = pieceWidth;
      if (pieceWidth >= style.width) {
        emitSegment(block, params, true, false);
      }
    } else {
      const bool startedFreshLine = segmentText.empty() && currentLineWidth == 0;
      segmentText += text;
      currentLineWidth += pieceWidth;
      if (startedFreshLine && pieceWidth >= style.width) {
        emitSegment(block, params, true, false);
      }
    }
  };

  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    auto fontFamily = block.fontFamily.value_or(props.fontFamily);
    auto fontSize = block.fontSize.value_or(props.fontSize);
    auto fontColor = block.fontColor.value_or(props.fontColor);
    auto fontName = TextLine::getFontNameFromFamily(fontFamily);

    sdl2w::RenderTextParams params;
    params.fontName = fontName.cStr();
    params.fontSize = ui::applyFontScale(fontSize, fontScale);
    params.color = fontColor;
    params.centered = false;

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
        emitSegment(block, params, true, true);
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

    // Mid-line TextBlock switch: flush this block's segment without ending the
    // visual line so later genBlocks can share the same lineNumber.
    if (!segmentText.empty()) {
      emitSegment(block, params, false, false);
    }
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
    bool currentLineIsBlank = true;

    auto flushLine = [&]() {
      if (currentLineBlocks.empty()) {
        return;
      }

      auto textLine = new TextLine(window, quad.get());
      textLine->setPos(props.padding, currentY);
      textLine->setScale(1.f);

      TextLineProps lineProps;
      lineProps.textBlocks = currentLineBlocks;
      lineProps.fontFamily =
          currentLineBlocks[0].fontFamily.value_or(props.fontFamily);
      lineProps.fontSize = currentLineBlocks[0].fontSize.value_or(props.fontSize);
      lineProps.fontColor = currentLineBlocks[0].fontColor.value_or(props.fontColor);
      lineProps.textAlign = props.textAlign;
      textLine->setProps(lineProps);

      quad->addChild(textLine);
      currentLineBlocks.clear();

      currentY += lineBoxAdvance(currentLineMaxHeight,
                                 currentLineIsBlank,
                                 props.lineHeightScale) +
                  props.lineSpacing;
      currentLineMaxHeight = 0;
      currentLineIsBlank = true;
    };

    for (const auto& genBlock : generatedBlocks) {
      if (genBlock.lineNumber != currentLineNumber) {
        if (currentLineNumber >= 0) {
          flushLine();
        }
        currentLineNumber = genBlock.lineNumber;
      }

      currentLineMaxHeight = std::max(currentLineMaxHeight, genBlock.textHeight);
      if (!genBlock.text.empty()) {
        currentLineIsBlank = false;
      }

      TextBlock textBlock;
      textBlock.text = genBlock.text;
      textBlock.fontFamily = genBlock.textBlock.fontFamily.value_or(props.fontFamily);
      textBlock.fontSize = genBlock.textBlock.fontSize.value_or(props.fontSize);
      textBlock.fontColor = genBlock.textBlock.fontColor.value_or(props.fontColor);
      currentLineBlocks.pushBack(textBlock);
    }

    if (!currentLineBlocks.empty()) {
      const auto& lastGenBlock = generatedBlocks.back();
      // Last line: place glyphs but do not advance — container height comes from
      // getContentHeight(), which reserves full glyph bounds for descenders.
      auto textLine = new TextLine(window, quad.get());
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
