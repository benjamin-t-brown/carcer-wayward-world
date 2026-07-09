#include "PageTalkChoice.h"
#include "ui/colors.h"
#include "ui/components/borders/BorderModalStandard.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/layouts/ModalStandard.h"

namespace ui {

PageTalkChoice::PageTalkChoice(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageTalkChoice::setProps(const PageTalkChoiceProps& _props) {
  props = _props;
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

  // Create ModalStandard layout
  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  auto& modalStyle = modal->getStyle();
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;

  ModalStandardProps modalProps;
  modal->setProps(modalProps);

  children.pushBack(bmin::UniquePtr<UiElement>(modal));

  auto [scaledContentW, scaledContentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  // auto choiceHeightScaled = props.choiceAreaHeight * style.scale;

  auto choiceSectionHeight = props.choiceAreaHeight;
  auto textSectionHeight =
      (scaledContentH / style.scale - BorderModalStandard::BOTTOM_BORDER_HEIGHT -
       choiceSectionHeight);
  auto borderHeight = BorderModalStandard::BOTTOM_BORDER_HEIGHT;
  auto scrollBarWidth = 32;

  // Create title element
  auto title = new TextLine(window, this);
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.fontSize = sdl2w::TEXT_SIZE_24;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = props.title;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  // Create SectionScrollable for content area
  auto textSection = new SectionScrollable(window, this);
  textSection->setId("textSection");
  auto& textSectionStyle = textSection->getStyle();
  textSectionStyle.width = scaledContentW / style.scale;
  textSectionStyle.height = textSectionHeight;
  textSectionStyle.x = contentX;
  textSectionStyle.y = contentY;
  textSectionStyle.scale = style.scale;
  textSection->setProps(SectionScrollableProps{.scrollBarWidth = scrollBarWidth});
  addChild(textSection);
  auto [textScrollableContentWidthScaled, _] = textSection->getContentDims();

  auto textBlock = new TextParagraph(window, textSection);
  textBlock->setId("textBlocks");
  auto& textBlockStyle = textBlock->getStyle();
  setBaseFontConfig(textBlockStyle, BaseFontConfig::MODAL_CHOICE_TEXT);
  textBlockStyle.lineSpacing = 0;
  textBlockStyle.x = 0;
  textBlockStyle.y = 0;
  textBlockStyle.width = textScrollableContentWidthScaled;
  textBlockStyle.scale = 1.f;
  textBlock->setProps(TextParagraphProps{
      .textBlocks = props.textBlocks, .bgColor = Colors::OffWhite, .padding = 4});
  textSection->addChild(textBlock);
  textSection->build();

  auto sepBorder = new OutsetRectangle(window, this);
  BaseStyle sepBorderStyle;
  sepBorderStyle.width = scaledContentW / style.scale;
  sepBorderStyle.height = 10;
  sepBorderStyle.x = contentX;
  sepBorderStyle.y = contentY + textSectionHeight * style.scale;
  sepBorderStyle.scale = style.scale;
  sepBorder->setStyle(sepBorderStyle);
  sepBorder->setProps(OutsetRectangleProps{});
  addChild(sepBorder);

  auto choiceSection = new SectionScrollable(window, this);
  choiceSection->setId("choiceSection");
  auto& choiceSectionStyle = choiceSection->getStyle();
  choiceSectionStyle.width = scaledContentW / style.scale;
  choiceSectionStyle.height = choiceSectionHeight;
  choiceSectionStyle.x = contentX;
  choiceSectionStyle.y = contentY + (textSectionHeight + borderHeight) * style.scale;
  choiceSectionStyle.scale = style.scale;
  choiceSection->setProps(
      SectionScrollableProps{.scrollBarWidth = scrollBarWidth, .indicatorHeight = 0});
  addChild(choiceSection);

  // Create choices
  for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
    auto choiceButton = new ButtonTextWrap(window, choiceSection);
    choiceButton->setId("choice" + bmin::toString(i));
    auto& choiceButtonStyle = choiceButton->getStyle();
    choiceButtonStyle.width =
        scaledContentW - 8 * style.scale - scrollBarWidth * style.scale;
    setBaseFontConfig(choiceButtonStyle, BaseFontConfig::MODAL_CHOICE_TEXT);
    choiceButtonStyle.scale = 1.f;
    choiceButtonStyle.fontColor = Colors::DarkBlue;
    ui::ButtonTextWrapProps choiceButtonProps;
    const bmin::String& prefixText = props.choices[i].prefixText;
    choiceButtonProps.text =
        bmin::toString(i + 1) + ". " +
        ((prefixText.empty() ? props.choices[i].text
                             : prefixText + " " + props.choices[i].text));
    choiceButtonProps.isSelected = false;
    choiceButton->setProps(choiceButtonProps);
    auto [choiceWidth, choiceHeight] = choiceButton->getDims();
    choiceButtonStyle.x = 4 * style.scale;
    choiceButtonStyle.y = i * choiceHeight;
    choiceButton->build();
    choiceSection->addChild(choiceButton);
  }

  choiceSection->build();
}

void PageTalkChoice::render(int dt) { UiElement::render(dt); }

} // namespace ui
