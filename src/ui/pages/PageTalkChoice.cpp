#include "PageTalkChoice.h"
#include "actions/navigation/UiContinueSpecialEvent.hpp"
#include "actions/navigation/UiSelectSpecialEventChoice.hpp"
#include "sdl2w/L10n.h"
#include "ui/FontScale.h"
#include "ui/colors.hpp"
#include "ui/components/borders/BorderModalStandard.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/observers/ActionObserver.hpp"
#include <algorithm>

namespace ui {

class PageTalkChoiceShowMoreObserver : public UiEventObserver {
  PageTalkChoice* page;

public:
  explicit PageTalkChoiceShowMoreObserver(PageTalkChoice* _page) : page(_page) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (page) {
      page->performShowMore();
    }
  }
};

bmin::DynArray<TextBlock>
PageTalkChoice::colorizeDialogueByQuotes(const bmin::DynArray<TextBlock>& blocks,
                                         SDL_Color outsideColor) {
  bmin::DynArray<TextBlock> result;
  for (const auto& source : blocks) {
    const auto& text = source.text;
    if (text.empty()) {
      continue;
    }

    bool inQuotes = false;
    size_t segmentStart = 0;
    auto emitSegment = [&](size_t end, bool quoted) {
      if (end <= segmentStart) {
        return;
      }
      TextBlock piece;
      piece.text = text.substr(segmentStart, end - segmentStart);
      piece.fontFamily = source.fontFamily;
      piece.fontSize = source.fontSize;
      if (source.fontColor.has_value()) {
        // Preserve caller-set colors (e.g. echoed player choices in history).
        piece.fontColor = source.fontColor;
      } else if (quoted) {
        piece.fontColor = Colors::Charcoal;
      } else {
        piece.fontColor = outsideColor;
      }
      result.pushBack(piece);
    };

    for (size_t i = 0; i < text.size(); i++) {
      if (text[i] != '"') {
        continue;
      }
      if (inQuotes) {
        emitSegment(i + 1, true);
        segmentStart = i + 1;
        inQuotes = false;
      } else {
        emitSegment(i, false);
        segmentStart = i;
        inQuotes = true;
      }
    }
    emitSegment(text.size(), inQuotes);
  }
  return result;
}

PageTalkChoice::PageTalkChoice(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  setupKeyboardScroll();
}

void PageTalkChoice::setProps(const PageTalkChoiceProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageTalkChoiceProps& PageTalkChoice::getProps() { return props; }

const PageTalkChoiceProps& PageTalkChoice::getProps() const { return props; }

const std::pair<int, int> PageTalkChoice::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageTalkChoice::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  // Create ModalStandard layout
  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  // FullBleed: LayerSpecialEvent talk path passes window dims and expects the dialogue
  // shell to fill the window (history + choice panes). CappedCentered would shrink the
  // talk UI on landscape and fight that layout.
  modal->setProps(ModalStandardProps{
      .width = style.width,
      .height = style.height,
      .layoutFit = LayoutFit::FullBleed,
      .iconSprite = props.portraitSpriteName,
      .portraitScale = props.portraitScale,
  });

  children.pushBack(bmin::UniquePtr<UiElement>(modal));

  if (!props.portraitSpriteName.empty()) {
    if (auto* border =
            dynamic_cast<BorderModalStandard*>(modal->getChildById("border"))) {
      auto [iconX, iconY] = border->getIconBorderLocation();
      const int iconSize = border->getProps().iconSize;
      auto iconBg = bmin::makeUnique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setPos(iconX, iconY);
      iconBg->setScale(style.scale);
      iconBg->setProps(QuadProps{
          .width = iconSize,
          .height = iconSize,
          .bgColor = Colors::OffWhite,
      });

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(modalChildren.begin(),
                                             modalChildren.end(),
                                             [](const bmin::UniquePtr<UiElement>& child) {
                                               return child->getId() == "headerIcon";
                                             });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
      } else {
        modal->addChild(bmin::UniquePtr<ui::UiElement>(iconBg.release()));
      }
    }
  }

  auto [scaledContentW, scaledContentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();

  // Content dims already exclude BorderModalStandard::BOTTOM_BORDER_HEIGHT chrome.
  // Reserve the 10px separator and thin footer inside the remaining content area.
  auto textSectionHeight =
      std::max(0,
               static_cast<int>(scaledContentH / style.scale) - SEP_BORDER_HEIGHT -
                   FOOTER_AREA_HEIGHT);
  auto scrollBarWidth = 32;

  // Create title element
  auto title = new TextLine(window, this);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = sdl2w::TEXT_SIZE_24;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = props.title;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(bmin::UniquePtr<ui::UiElement>(title));

  // Create SectionScrollable for content area
  auto textSection = new SectionScrollable(window, this);
  textSection->setId("textSection");
  textSection->setPos(contentX, contentY);
  textSection->setScale(style.scale);
  textSection->setProps(SectionScrollableProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = static_cast<int>(textSectionHeight),
      .scrollBarWidth = scrollBarWidth,
  });
  addChild(bmin::UniquePtr<ui::UiElement>(textSection));
  auto [textScrollableContentWidthScaled, textViewportHeightScaled] =
      textSection->getContentDims();

  TextFontProps textFont;
  setBaseFontConfig(textFont, BaseFontConfig::MODAL_CHOICE_TEXT);

  const int pinFrom =
      std::clamp(props.pinFromBlockIndex, 0, static_cast<int>(props.textBlocks.size()));
  bmin::DynArray<TextBlock> historyBlocksRaw;
  bmin::DynArray<TextBlock> currentDialogueRaw;
  bmin::DynArray<TextBlock> currentSystemRaw;
  for (int i = 0; i < static_cast<int>(props.textBlocks.size()); i++) {
    const auto& block = props.textBlocks[i];
    if (i < pinFrom) {
      historyBlocksRaw.pushBack(block);
    } else if (block.fontColor.has_value()) {
      if (currentSystemRaw.empty() ||
          currentSystemRaw[currentSystemRaw.size() - 1].text != block.text) {
        currentSystemRaw.pushBack(block);
      }
    } else {
      currentDialogueRaw.pushBack(block);
    }
  }
  const auto historyBlocks = colorizeDialogueByQuotes(historyBlocksRaw, Colors::Grey2);
  const auto currentBlocks = colorizeDialogueByQuotes(currentDialogueRaw, Colors::Grey2);

  int contentYOffset = 0;
  int historyHeightScaled = 0;

  auto addParagraph = [&](const bmin::DynArray<TextBlock>& blocks,
                          const bmin::String& id) -> TextParagraph* {
    if (blocks.empty()) {
      return nullptr;
    }
    auto* paragraph = new TextParagraph(window, textSection);
    paragraph->setId(id);
    paragraph->setPos(0, contentYOffset);
    paragraph->setScale(1.f);
    paragraph->setProps(TextParagraphProps{
        .textBlocks = blocks,
        .width = textScrollableContentWidthScaled,
        .bgColor = Colors::OffWhite,
        .padding = 4,
        .lineSpacing = 0,
        .lineHeightScale = props.lineHeightScale,
        .blankLineHeightScale = props.blankLineHeightScale,
        .fontFamily = textFont.fontFamily,
        .fontSize = textFont.fontSize,
        .fontColor = textFont.fontColor,
    });
    textSection->addChild(bmin::UniquePtr<ui::UiElement>(paragraph));
    const int height = paragraph->getDims().second;
    contentYOffset += height;
    return paragraph;
  };

  if (auto* historyParagraph = addParagraph(historyBlocks, "textBlocksHistory")) {
    historyHeightScaled = historyParagraph->getDims().second;
  }

  int currentHeightScaled = 0;
  if (auto* currentParagraph = addParagraph(currentBlocks, "textBlocks")) {
    currentHeightScaled = currentParagraph->getDims().second;
  }
  if (auto* journalParagraph = addParagraph(currentSystemRaw, "textBlocksJournal")) {
    currentHeightScaled += journalParagraph->getDims().second;
  }

  if (!props.choices.empty() && contentYOffset > 0) {
    int fontScale = 0;
    if (auto* stateManager = getStateManager()) {
      fontScale = stateManager->getState().settings.fontScale;
    }
    const auto fontName = TextLine::getFontNameFromFamily(textFont.fontFamily);
    sdl2w::RenderTextParams measureParams;
    measureParams.fontName = fontName.cStr();
    measureParams.fontSize = ui::applyFontScale(textFont.fontSize, fontScale);
    const int lineH = window->getDraw().measureText(" ", measureParams).second;
    const int choiceGapScaled =
        std::max(1, static_cast<int>(lineH * props.lineHeightScale));
    auto* gap = new Quad(window, textSection);
    gap->setId("textChoiceGap");
    gap->setPos(0, contentYOffset);
    gap->setScale(1.f);
    gap->setProps(QuadProps{
        .width = textScrollableContentWidthScaled,
        .height = choiceGapScaled,
        .bgColor = Colors::OffWhite,
    });
    textSection->addChild(bmin::UniquePtr<ui::UiElement>(gap));
    contentYOffset += choiceGapScaled;
    currentHeightScaled += choiceGapScaled;
  }

  // Authored choices live in the log after current dialogue (not a reserved pane).
  int choicesHeightScaled = 0;
  auto choiceYOffset = contentYOffset;
  for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
    auto choiceButton = new ButtonTextWrap(window, textSection);
    choiceButton->setId("choice" + bmin::toString(i));
    TextFontProps choiceFont;
    setBaseFontConfig(choiceFont, BaseFontConfig::MODAL_CHOICE_TEXT);
    ui::ButtonTextWrapProps choiceButtonProps;
    const bmin::String& prefixText = props.choices[i].prefixText;
    const bmin::String choiceText =
        " " + bmin::toString(i + 1) + ". " +
        ((prefixText.empty() ? props.choices[i].text
                             : prefixText + " " + props.choices[i].text));
    const SDL_Color choiceColor =
        props.choices[i].previouslyChosen ? Colors::Grey : Colors::DarkBlue;
    choiceButtonProps.isSelected = false;
    choiceButtonProps.textParagraph.textBlocks.pushBack(
        TextBlock{.text = choiceText, .fontColor = choiceColor});
    choiceButtonProps.textParagraph.width = textScrollableContentWidthScaled - 8;
    choiceButtonProps.textParagraph.fontFamily = choiceFont.fontFamily;
    choiceButtonProps.textParagraph.fontSize = choiceFont.fontSize;
    choiceButtonProps.textParagraph.fontColor = choiceColor;
    choiceButtonProps.textParagraph.lineHeightScale = 0.85f;
    choiceButtonProps.textParagraph.bgColor = Colors::OffWhite;
    choiceButtonProps.bgColor = Colors::OffWhite;
    choiceButtonProps.verticalPadding = 0;
    choiceButton->setScale(1.f);
    choiceButton->setPos(4, choiceYOffset);
    choiceButton->setProps(choiceButtonProps);
    choiceButton->addEventObserver(
        ui::makeActionObserver<state::actions::UiSelectSpecialEventChoice>(i));
    const int choiceHeight = choiceButton->getDims().second;
    textSection->addChild(bmin::UniquePtr<ui::UiElement>(choiceButton));
    choiceYOffset += choiceHeight;
  }
  choicesHeightScaled = choiceYOffset - contentYOffset;
  contentYOffset = choiceYOffset;

  // Pad so the pinned stop (current dialogue + notices + in-flow choices) can sit at
  // the top of the viewport.
  const int currentStopHeightScaled = currentHeightScaled + choicesHeightScaled;
  const int padHeightScaled =
      std::max(0, textViewportHeightScaled - currentStopHeightScaled);
  if (padHeightScaled > 0) {
    auto* spacer = new Quad(window, textSection);
    spacer->setId("textBottomPad");
    spacer->setPos(0, contentYOffset);
    spacer->setScale(1.f);
    spacer->setProps(QuadProps{
        .width = textScrollableContentWidthScaled,
        .height = padHeightScaled,
        .bgColor = Colors::OffWhite,
    });
    textSection->addChild(bmin::UniquePtr<ui::UiElement>(spacer));
  }

  textSection->build();
  textSection->scrollTo(historyHeightScaled);

  auto sepBorder = new OutsetRectangle(window, this);
  sepBorder->setPos(contentX, contentY + textSectionHeight * style.scale);
  sepBorder->setScale(style.scale);
  sepBorder->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = SEP_BORDER_HEIGHT,
  });
  addChild(bmin::UniquePtr<ui::UiElement>(sepBorder));

  const int buttonPadding = 2;
  const int buttonWidth = 120;
  auto buttonGroup = new ButtonGroup(window, this);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(contentX,
                      contentY + (textSectionHeight + SEP_BORDER_HEIGHT) * style.scale);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(ButtonGroupProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .alignment = ButtonGroupAlignment::CENTER,
      .buttonWidth = buttonWidth,
      .buttonHeight = FOOTER_AREA_HEIGHT - 2 * buttonPadding,
      .padding = buttonPadding,
      // Continue / Show More / inert modes fill in after layout via syncFooter.
      .buttons = {},
  });
  addChild(bmin::UniquePtr<ui::UiElement>(buttonGroup));
  syncFooter(true);
  footerNeedsSync = false;
}

ButtonTextWrap* PageTalkChoice::choiceButton(int i) {
  if (i < 0) {
    return nullptr;
  }
  const auto choiceId = "choice" + bmin::toString(i);
  return dynamic_cast<ButtonTextWrap*>(getChildById(choiceId.cStr()));
}

SectionScrollable* PageTalkChoice::textSection() {
  return dynamic_cast<SectionScrollable*>(getChildById("textSection"));
}

void PageTalkChoice::render(int dt) {
  UiElement::render(dt);
  renderShowMoreCue();
}

void PageTalkChoice::setupKeyboardScroll() {
  keyboardScroll.clearBindings();

  // Unified log: all arrows (and numpad) scroll textSection.
  auto text = [this]() { return textSection(); };

  keyboardScroll.bindSectionKey("Left", text, ScrollDirection::Up);
  keyboardScroll.bindSectionKey("Keypad 4", text, ScrollDirection::Up);
  keyboardScroll.bindSectionKey("Right", text, ScrollDirection::Down);
  keyboardScroll.bindSectionKey("Keypad 6", text, ScrollDirection::Down);
  keyboardScroll.bindSectionKey("Up", text, ScrollDirection::Up);
  keyboardScroll.bindSectionKey("Keypad 8", text, ScrollDirection::Up);
  keyboardScroll.bindSectionKey("Down", text, ScrollDirection::Down);
  keyboardScroll.bindSectionKey("Keypad 2", text, ScrollDirection::Down);
}

void PageTalkChoice::enqueueSelectChoice(int choiceIndex) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiSelectSpecialEventChoice>(choiceIndex), 0);
}

void PageTalkChoice::enqueueContinue() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiContinueSpecialEvent>(), 0);
}

ButtonGroup* PageTalkChoice::footerButtonGroup() {
  return dynamic_cast<ButtonGroup*>(getChildById("buttonGroup"));
}

ButtonModal* PageTalkChoice::footerButton() {
  auto* group = footerButtonGroup();
  if (!group || group->getChildren().empty()) {
    return nullptr;
  }
  return dynamic_cast<ButtonModal*>(group->getChildren()[0].get());
}

bool PageTalkChoice::isChoiceClipped(int choiceIndex) {
  auto* section = textSection();
  auto* choice = choiceButton(choiceIndex);
  if (!section || !choice) {
    return false;
  }
  const auto choiceTop = choice->getPos().second;
  const auto choiceHeight = choice->getDims().second;
  const auto choiceBottom = choiceTop + choiceHeight;
  const auto viewportBottom =
      section->getScrollOffset() + section->getContentDims().second;
  return choiceBottom > viewportBottom;
}

int PageTalkChoice::firstClippedChoiceIndex() {
  for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
    if (isChoiceClipped(i)) {
      return i;
    }
  }
  return -1;
}

PageTalkChoice::FooterMode PageTalkChoice::computeFooterMode() {
  if (props.showContinue && props.choices.empty()) {
    return FooterMode::Continue;
  }
  if (props.choices.empty()) {
    return FooterMode::Inert;
  }
  if (isChoiceClipped(static_cast<int>(props.choices.size()) - 1)) {
    return FooterMode::ShowMore;
  }
  return FooterMode::Inert;
}

void PageTalkChoice::retargetFooter(FooterMode mode) {
  auto* group = footerButtonGroup();
  if (!group) {
    footerMode = mode;
    return;
  }

  auto groupProps = group->getProps();
  groupProps.alignment = ButtonGroupAlignment::CENTER;
  switch (mode) {
  case FooterMode::Continue:
    groupProps.buttonWidth = 120;
    groupProps.buttons = {
        {.label = TRANSLATE("Continue"), .type = ButtonGroupButtonType::MODAL}};
    break;
  case FooterMode::ShowMore:
    groupProps.buttonWidth = 160;
    groupProps.buttons = {
        {.label = TRANSLATE("Show More"), .type = ButtonGroupButtonType::MODAL}};
    break;
  case FooterMode::Inert:
    groupProps.buttons = {};
    break;
  }
  group->setProps(groupProps);

  if (mode == FooterMode::Continue) {
    group->addObserverToButtonAtIndex(
        0, ui::makeActionObserver<state::actions::UiContinueSpecialEvent>());
  } else if (mode == FooterMode::ShowMore) {
    group->addObserverToButtonAtIndex(
        0,
        bmin::UniquePtr<UiEventObserver>(new PageTalkChoiceShowMoreObserver(this)));
    styleShowMoreButton();
  }

  footerMode = mode;
}

void PageTalkChoice::styleShowMoreButton() {
  auto* button = footerButton();
  if (!button) {
    return;
  }
  auto buttonProps = button->getProps();
  buttonProps.bgColor = Colors::ButtonShowMore;
  buttonProps.bgColorTopRight = Colors::ButtonShowMoreLight;
  buttonProps.bgColorBottomLeft = Colors::ButtonShowMoreDark;
  button->setProps(buttonProps);
}

void PageTalkChoice::renderShowMoreCue() {
  if (props.choices.empty() ||
      !isChoiceClipped(static_cast<int>(props.choices.size()) - 1)) {
    return;
  }

  auto* section = textSection();
  if (!section) {
    return;
  }

  const auto [sectionX, sectionY] = section->getPos();
  const int sectionW = section->getDims().first;
  const auto [contentW, contentH] = section->getContentDims();
  if (contentW <= 0 || contentH <= 0 || sectionW <= 0) {
    return;
  }

  auto& draw = window->getDraw();
  const int vignetteH = std::min(72, contentH);
  constexpr int kBands = 10;
  const int bandH = std::max(1, vignetteH / kBands);
  const int fadeTop = sectionY + contentH - vignetteH;
  for (int i = 0; i < kBands; ++i) {
    const auto alpha = static_cast<Uint8>(((i + 1) * 230) / kBands);
    draw.drawRect(sectionX,
                  fadeTop + i * bandH,
                  contentW,
                  bandH,
                  SDL_Color{Colors::OffWhite.r, Colors::OffWhite.g, Colors::OffWhite.b,
                            alpha});
  }

  // Match Continue / Show More, which are centered on the full modal content
  // (text pane + scrollbar), not the text column alone.
  const int centerX = sectionX + sectionW / 2;
  const int centerY = sectionY + contentH - 18;
  const int arrow = 10;
  const float stroke = std::max(2.f, style.scale);
  draw.drawLine({centerX - arrow, centerY - 5},
                {centerX, centerY + 6},
                stroke,
                Colors::Grey2);
  draw.drawLine({centerX + arrow, centerY - 5},
                {centerX, centerY + 6},
                stroke,
                Colors::Grey2);
}

void PageTalkChoice::syncFooter(bool force) {
  const auto mode = computeFooterMode();
  if (!force && mode == footerMode) {
    return;
  }
  retargetFooter(mode);
}

void PageTalkChoice::performShowMore() {
  auto* section = textSection();
  const auto clippedIndex = firstClippedChoiceIndex();
  auto* choice = choiceButton(clippedIndex);
  if (section && choice) {
    section->scrollTo(choice->getPos().second);
  }
  // Defer footer rebuild until after click dispatch / keyboard flash callback.
  footerNeedsSync = true;
}

void PageTalkChoice::beginKeyboardChoicePress(int choiceIndex) {
  if (keyboardFlash.isBusy()) {
    return;
  }
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= props.choices.size()) {
    return;
  }
  keyboardFlash.begin(
      [this, choiceIndex]() -> bool* {
        if (auto* button = choiceButton(choiceIndex)) {
          return &button->isActive;
        }
        return nullptr;
      },
      [this, choiceIndex]() { enqueueSelectChoice(choiceIndex); },
      window);
}

void PageTalkChoice::beginKeyboardContinuePress() {
  if (keyboardFlash.isBusy()) {
    return;
  }
  if (footerMode != FooterMode::Continue && footerMode != FooterMode::ShowMore) {
    return;
  }
  const auto shouldContinue = footerMode == FooterMode::Continue;
  keyboardFlash.begin(
      [this]() -> bool* {
        if (auto* button = footerButton()) {
          return &button->isActive;
        }
        return nullptr;
      },
      [this, shouldContinue]() {
        if (shouldContinue) {
          enqueueContinue();
        } else {
          performShowMore();
        }
      },
      window);
}

void PageTalkChoice::onKeyDown(std::string_view key) {
  if (keyboardScroll.onKeyDown(key)) {
    return;
  }

  if (KeyboardPressFlash::isConfirmKey(key)) {
    keyboardScroll.stopScroll();
    beginKeyboardContinuePress();
    return;
  }

  if (const auto choiceIndex = KeyboardPressFlash::choiceIndexFromKey(key)) {
    keyboardScroll.stopScroll();
    beginKeyboardChoicePress(*choiceIndex);
  }
}

void PageTalkChoice::onKeyUp(std::string_view key) { keyboardScroll.onKeyUp(key); }

void PageTalkChoice::updateKeyboardChrome(int deltaTime) {
  keyboardScroll.update(deltaTime, window);
  keyboardFlash.update(deltaTime);
  if (footerNeedsSync) {
    footerNeedsSync = false;
    syncFooter(false);
  } else if (!keyboardFlash.isBusy()) {
    // Wheel / arrow scroll can reveal the last choice; keep the footer in sync.
    syncFooter(false);
  }
}

void PageTalkChoice::stopKeyboardChrome() {
  keyboardScroll.stopScroll();
  keyboardFlash.stop();
}

} // namespace ui
