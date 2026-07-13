#include "PageModalEvent.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/borders/BorderModalSmall.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/layouts/ModalSmall.h"

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
  addChild(modal);

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
  modal->setTitleElement(title);

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
  auto [scrollableContentW, _] = scrollableSection->getContentDims();

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
  scrollableSection->addChild(textBlock);

  int choiceYOffset = 0;
  if (!props.choices.empty()) {
    auto [_, textHeight] = textBlock->getDims();
    choiceYOffset = textHeight;
    for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
      auto choiceButton = new ButtonTextWrap(window, scrollableSection);
      choiceButton->setId("choice" + bmin::toString(i));
      TextFontProps choiceFont;
      setBaseFontConfig(choiceFont, BaseFontConfig::MODAL_CHOICE_TEXT);
      ButtonTextWrapProps choiceButtonProps;
      const bmin::String& prefixText = props.choices[i].prefixText;
      choiceButtonProps.text =
          bmin::toString(i + 1) + ". " +
          (prefixText.empty() ? props.choices[i].text
                              : prefixText + " " + props.choices[i].text);
      choiceButtonProps.width = scrollableContentW - 8;
      choiceButtonProps.fontFamily = choiceFont.fontFamily;
      choiceButtonProps.fontSize = choiceFont.fontSize;
      choiceButtonProps.fontColor = Colors::DarkBlue;
      choiceButton->setScale(1.f);
      choiceButton->setProps(choiceButtonProps);
      auto [__, choiceHeight] = choiceButton->getDims();
      choiceButton->setPos(4, choiceYOffset);
      choiceYOffset += choiceHeight;
      scrollableSection->addChild(choiceButton);
    }
  }

  scrollableSection->build();
  modal->addChild(scrollableSection);

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
        .buttons = {{.label = TRANSLATE("Continue"), .type = ButtonGroupButtonType::MODAL}},
    });
    modal->addChild(buttonGroup);
  }
}

void PageModalEvent::render(int dt) { UiElement::render(dt); }

} // namespace ui
