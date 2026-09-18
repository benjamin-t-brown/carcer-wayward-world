#include "PageModalEvent.h"
#include "actions/navigation/UiContinueSpecialEvent.hpp"
#include "actions/navigation/UiSelectSpecialEventChoice.hpp"
#include "sdl2w/L10n.h"
#include "ui/colors.hpp"
#include "ui/components/borders/BorderModalSmall.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/helpers/modalLayoutFit.h"
#include "ui/layouts/ModalSmall.h"
#include "ui/observers/ActionObserver.hpp"
#include <algorithm>

namespace ui {

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

  auto* border = dynamic_cast<BorderModalSmall*>(modal->getChildById("border"));
  auto [contentX, contentY] = modal->getContentLocation();
  int contentW = 0;
  int contentH = 0;
  if (props.showContinueButton && props.choices.empty()) {
    // Reserve the bottom button strip for Continue.
    auto [w, h] = modal->getContentDims();
    contentW = w;
    contentH = h;
  } else if (border) {
    // No Continue row — use the full content area (choices live in the scroll body).
    auto [w, h] = border->getContentDims();
    contentW = w;
    contentH = h;
  }

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

  int contentBottom = textBlock->getDims().second;
  if (!props.choices.empty()) {
    int choiceYOffset = contentBottom;
    for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
      auto choiceButton = new ButtonTextWrap(window, scrollableSection);
      choiceButton->setId("choice" + bmin::toString(i));
      TextFontProps choiceFont;
      setBaseFontConfig(choiceFont, BaseFontConfig::MODAL_CHOICE_TEXT);
      ButtonTextWrapProps choiceButtonProps;
      const bmin::String& prefixText = props.choices[i].prefixText;
      const bmin::String choiceText =
          bmin::toString(i + 1) + ". " +
          (prefixText.empty() ? props.choices[i].text
                              : prefixText + " " + props.choices[i].text);
      const SDL_Color choiceColor =
          props.choices[i].previouslyChosen ? Colors::Grey : Colors::DarkBlue;
      choiceButtonProps.textParagraph.textBlocks.pushBack(
          TextBlock{.text = choiceText, .fontColor = choiceColor});
      choiceButtonProps.textParagraph.width = scrollableContentW - 8;
      choiceButtonProps.textParagraph.fontFamily = choiceFont.fontFamily;
      choiceButtonProps.textParagraph.fontSize = choiceFont.fontSize;
      choiceButtonProps.textParagraph.fontColor = choiceColor;
      choiceButton->setScale(1.f);
      choiceButton->setPos(4, choiceYOffset);
      choiceButton->setProps(choiceButtonProps);
      choiceButton->addEventObserver(
          ui::makeActionObserver<state::actions::UiSelectSpecialEventChoice>(i));
      auto [__, choiceHeight] = choiceButton->getDims();
      choiceYOffset += choiceHeight;
      scrollableSection->addChild(bmin::UniquePtr<ui::UiElement>(choiceButton));
    }
    contentBottom = choiceYOffset;
  }

  // Fill remaining viewport so short text reaches the button strip (talk modal pattern).
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

  if (props.showContinueButton && props.choices.empty()) {
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
        .buttons = {{.label = TRANSLATE("Okay"), .type = ButtonGroupButtonType::MODAL}},
    });
    buttonGroup->addObserverToButtonAtIndex(
        0, ui::makeActionObserver<state::actions::UiContinueSpecialEvent>());
    modal->addChild(bmin::UniquePtr<ui::UiElement>(buttonGroup));
  }
}

ButtonTextWrap* PageModalEvent::choiceButton(int i) {
  if (i < 0) {
    return nullptr;
  }
  const auto choiceId = "choice" + bmin::toString(i);
  return dynamic_cast<ButtonTextWrap*>(getChildById(choiceId.cStr()));
}

ButtonModal* PageModalEvent::continueButton() {
  auto* buttonGroup = dynamic_cast<ButtonGroup*>(getChildById("buttonGroup"));
  if (!buttonGroup || buttonGroup->getChildren().empty()) {
    return nullptr;
  }
  return dynamic_cast<ButtonModal*>(buttonGroup->getChildren()[0].get());
}

void PageModalEvent::render(int dt) { UiElement::render(dt); }

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
      [this, choiceIndex]() { enqueueSelectChoice(choiceIndex); });
}

void PageModalEvent::beginKeyboardContinuePress() {
  if (keyboardFlash.isBusy()) {
    return;
  }
  if (!props.choices.empty()) {
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
        [this]() { enqueueContinue(); });
    return;
  }
  // No Okay on terminal "End." — Enter still dismisses via the continue action.
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
}

void PageModalEvent::stopKeyboardChrome() { keyboardFlash.stop(); }

} // namespace ui
