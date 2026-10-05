#include "PageModalEvent.h"
#include "actions/navigation/UiContinueSpecialEvent.hpp"
#include "actions/navigation/UiSelectSpecialEventChoice.hpp"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include "sdl2w/Draw.h"
#include "sdl2w/L10n.h"
#include "ui/colors.hpp"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/helpers/modalLayoutFit.h"
#include "ui/helpers/uiSounds.h"
#include "ui/layouts/ModalSmall.h"
#include "ui/observers/ActionObserver.hpp"
#include <algorithm>

namespace ui {

class PageModalEventShowMoreObserver : public UiEventObserver {
  PageModalEvent* page;

public:
  explicit PageModalEventShowMoreObserver(PageModalEvent* _page) : page(_page) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (page) {
      page->performShowMore();
    }
  }
};

PageModalEvent::PageModalEvent(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageModalEvent::setProps(const PageModalEventProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageModalEventProps& PageModalEvent::getProps() { return props; }

const PageModalEventProps& PageModalEvent::getProps() const { return props; }

const std::pair<int, int> PageModalEvent::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageModalEvent::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
      .enableCloseButton = false,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);
  addChild(bmin::UniquePtr<ui::UiElement>(modal));

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  titleProps.textBlocks.pushBack({.text = props.title});
  title->setProps(titleProps);
  modal->setTitleElement(bmin::UniquePtr<ui::UiElement>(title));

  auto [contentX, contentY] = modal->getContentLocation();
  // Always reserve the button strip for Okay / Show More / choice buttons.
  auto [contentW, contentH] = modal->getContentDims();

  const int unscaledContentW = static_cast<int>(contentW / style.scale);
  const int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("textSection");
  scrollableSection->setPos(contentX, contentY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = unscaledContentH,
  });
  auto [scrollableContentW, scrollableViewportH] = scrollableSection->getContentDims();

  auto textBlock = new TextParagraph(window, scrollableSection);
  textBlock->setId("textBlocks");
  TextFontProps textFont;
  setBaseFontConfig(textFont, BaseFontConfig::MODAL_TEXT);
  textBlock->setPos(0, 0);
  textBlock->setScale(1.f);
  textBlock->setProps(TextParagraphProps{
      .textBlocks = props.textBlocks,
      .width = scrollableContentW,
      .bgColor = Colors::OffWhite,
      .padding = 4,
      .lineSpacing = 0,
      .fontFamily = textFont.fontFamily,
      .fontSize = textFont.fontSize,
      .fontColor = Colors::Black,
  });
  scrollableSection->addChild(bmin::UniquePtr<ui::UiElement>(textBlock));

  const int contentBottom = textBlock->getDims().second;

  // Fill remaining viewport so short text reaches the button strip.
  const int padHeight = std::max(0, scrollableViewportH - contentBottom);
  if (padHeight > 0) {
    auto* spacer = new Quad(window, scrollableSection);
    spacer->setId("textBottomPad");
    spacer->setPos(0, contentBottom);
    spacer->setScale(1.f);
    spacer->setProps(QuadProps{
        .width = scrollableContentW,
        .height = padHeight,
        .bgColor = Colors::OffWhite,
    });
    scrollableSection->addChild(bmin::UniquePtr<ui::UiElement>(spacer));
  }

  scrollableSection->build();
  modal->addChild(bmin::UniquePtr<ui::UiElement>(scrollableSection));

  auto [buttonsW, buttonsH] = modal->getButtonsDims();
  auto [buttonsX, buttonsY] = modal->getButtonsLocation();
  const int buttonPadding = 2;
  const int buttonWidth = 120;

  auto buttonGroup = new ButtonGroup(window, modal);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(buttonsX, buttonsY);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(ButtonGroupProps{
      .width = static_cast<int>(buttonsW / style.scale),
      .alignment = ButtonGroupAlignment::RIGHT,
      .buttonWidth = buttonWidth,
      .buttonHeight = ModalSmall::BUTTONS_AREA_HEIGHT - 2 * buttonPadding,
      .padding = buttonPadding,
      .buttons = {},
  });
  modal->addChild(bmin::UniquePtr<ui::UiElement>(buttonGroup));
  syncFooter(true);
  footerNeedsSync = false;
}

ButtonModal* PageModalEvent::choiceButton(int i) {
  auto* buttonGroup = footerButtonGroup();
  if (!buttonGroup || i < 0 || footerMode != FooterMode::Choices) {
    return nullptr;
  }
  const int count = static_cast<int>(props.choices.size());
  if (i >= count) {
    return nullptr;
  }
  // Choices are pushed in reverse so RIGHT alignment still reads left→right.
  const auto buttonId = "buttonGroupButton_" + bmin::toString(count - 1 - i);
  return dynamic_cast<ButtonModal*>(buttonGroup->getChildById(bmin::toStringView(buttonId)));
}

ButtonModal* PageModalEvent::continueButton() {
  auto* buttonGroup = footerButtonGroup();
  if (!buttonGroup || buttonGroup->getChildren().empty()) {
    return nullptr;
  }
  if (footerMode != FooterMode::Continue && footerMode != FooterMode::ShowMore) {
    return nullptr;
  }
  return dynamic_cast<ButtonModal*>(buttonGroup->getChildren()[0].get());
}

SectionScrollable* PageModalEvent::textSection() {
  return dynamic_cast<SectionScrollable*>(getChildById("textSection"));
}

void PageModalEvent::render(int dt) {
  UiElement::render(dt);
  renderShowMoreCue();
}

void PageModalEvent::enqueueSelectChoice(int choiceIndex) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiSelectSpecialEventChoice>(choiceIndex), 0);
}

void PageModalEvent::enqueueContinue() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiContinueSpecialEvent>(), 0);
}

void PageModalEvent::beginKeyboardChoicePress(int choiceIndex) {
  if (keyboardFlash.isBusy()) {
    return;
  }
  if (footerMode != FooterMode::Choices) {
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

void PageModalEvent::beginKeyboardContinuePress() {
  if (keyboardFlash.isBusy()) {
    return;
  }
  if (footerMode == FooterMode::ShowMore) {
    keyboardFlash.begin(
        [this]() -> bool* {
          if (auto* button = continueButton()) {
            return &button->isActive;
          }
          return nullptr;
        },
        [this]() { performShowMore(); },
        window);
    return;
  }
  if (footerMode == FooterMode::Choices) {
    return;
  }
  if (props.showContinueButton) {
    keyboardFlash.begin(
        [this]() -> bool* {
          if (auto* button = continueButton()) {
            return &button->isActive;
          }
          return nullptr;
        },
        [this]() { enqueueContinue(); },
        window);
    return;
  }
  // No Okay on terminal "End." — Enter still dismisses via the continue action.
  playButtonSound(window);
  enqueueContinue();
}

void PageModalEvent::onKeyDown(std::string_view key) {
  if (KeyboardPressFlash::isConfirmKey(key)) {
    beginKeyboardContinuePress();
    return;
  }
  if (const auto choiceIndex = KeyboardPressFlash::choiceIndexFromKey(key)) {
    beginKeyboardChoicePress(*choiceIndex);
  }
}

void PageModalEvent::onKeyUp(std::string_view /*key*/) {}

void PageModalEvent::updateKeyboardChrome(int deltaTime) {
  keyboardFlash.update(deltaTime);
  if (footerNeedsSync) {
    footerNeedsSync = false;
    syncFooter(false);
  } else if (!keyboardFlash.isBusy()) {
    // Wheel scroll can reveal the rest of the text; keep the footer in sync.
    syncFooter(false);
  }
}

void PageModalEvent::stopKeyboardChrome() { keyboardFlash.stop(); }

ButtonGroup* PageModalEvent::footerButtonGroup() {
  return dynamic_cast<ButtonGroup*>(getChildById("buttonGroup"));
}

bool PageModalEvent::isContentClipped() {
  auto* section = textSection();
  if (!section) {
    return false;
  }
  return section->getScrollOffset() < section->getMaxScrollOffset();
}

PageModalEvent::FooterMode PageModalEvent::computeFooterMode() {
  if (isContentClipped()) {
    return FooterMode::ShowMore;
  }
  if (!props.choices.empty()) {
    return FooterMode::Choices;
  }
  if (props.showContinueButton) {
    return FooterMode::Continue;
  }
  return FooterMode::Inert;
}

void PageModalEvent::retargetFooter(FooterMode mode) {
  auto* group = footerButtonGroup();
  if (!group) {
    footerMode = mode;
    return;
  }

  auto groupProps = group->getProps();
  groupProps.alignment = ButtonGroupAlignment::RIGHT;
  switch (mode) {
  case FooterMode::Continue:
    groupProps.buttonWidth = 120;
    groupProps.buttons = {
        {.label = TRANSLATE("Okay"), .type = ButtonGroupButtonType::MODAL}};
    break;
  case FooterMode::ShowMore:
    groupProps.buttonWidth = 160;
    groupProps.buttons = {
        {.label = TRANSLATE("Show More"), .type = ButtonGroupButtonType::MODAL}};
    break;
  case FooterMode::Choices: {
    const int count = static_cast<int>(props.choices.size());
    const int spacing = groupProps.buttonSpacing;
    const int padding = groupProps.padding;
    const int avail = std::max(1, groupProps.width - 2 * padding);
    int buttonWidth = 120;
    if (count > 0) {
      buttonWidth = (avail - (count - 1) * spacing) / count;
      buttonWidth = std::clamp(buttonWidth, 48, 160);
    }
    groupProps.buttonWidth = buttonWidth;
    groupProps.buttons.clear();
    // RIGHT packs index 0 on the far right — push last→first so choice 1
    // still reads on the left of the right-aligned group.
    for (int i = count - 1; i >= 0; i--) {
      const bmin::String& prefixText = props.choices[i].prefixText;
      const bmin::String label =
          prefixText.empty() ? props.choices[i].text
                             : prefixText + " " + props.choices[i].text;
      groupProps.buttons.pushBack(
          {.label = label, .type = ButtonGroupButtonType::MODAL});
    }
    break;
  }
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
        bmin::UniquePtr<UiEventObserver>(new PageModalEventShowMoreObserver(this)));
    styleShowMoreButton();
  } else if (mode == FooterMode::Choices) {
    const int count = static_cast<int>(props.choices.size());
    for (int buttonIndex = 0; buttonIndex < count; buttonIndex++) {
      const int choiceIndex = count - 1 - buttonIndex;
      group->addObserverToButtonAtIndex(
          buttonIndex,
          ui::makeActionObserver<state::actions::UiSelectSpecialEventChoice>(
              choiceIndex));
    }
  }

  footerMode = mode;
}

void PageModalEvent::styleShowMoreButton() {
  auto* button = continueButton();
  if (!button) {
    return;
  }
  auto buttonProps = button->getProps();
  buttonProps.bgColor = Colors::ButtonShowMore;
  buttonProps.bgColorTopRight = Colors::ButtonShowMoreLight;
  buttonProps.bgColorBottomLeft = Colors::ButtonShowMoreDark;
  button->setProps(buttonProps);
}

void PageModalEvent::renderShowMoreCue() {
  if (!isContentClipped()) {
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

void PageModalEvent::syncFooter(bool force) {
  const auto mode = computeFooterMode();
  if (!force && mode == footerMode) {
    return;
  }
  retargetFooter(mode);
}

void PageModalEvent::performShowMore() {
  auto* section = textSection();
  if (!section) {
    return;
  }
  const auto viewportH = section->getContentDims().second;
  section->scrollTo(section->getScrollOffset() + viewportH);
  footerNeedsSync = true;
}

} // namespace ui
