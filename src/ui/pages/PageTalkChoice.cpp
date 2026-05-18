#include "PageTalkChoice.h"
#include "ui/colors.h"
#include "ui/components/BorderModalStandard.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include <memory>

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
  BaseStyle modalStyle;
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setStyle(modalStyle);

  ModalStandardProps modalProps;
  modal->setProps(modalProps);

  children.push_back(std::unique_ptr<ModalStandard>(modal));

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLoc();
  auto choiceHeightScaled = props.choiceAreaHeight * style.scale;

  // Create title element
  auto title = std::make_unique<TextLine>(window, this);
  BaseStyle titleStyle;
  titleStyle.fontFamily = FontFamily::H2;
  titleStyle.fontSize = sdl2w::TEXT_SIZE_20;
  titleStyle.fontColor = Colors::White;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  title->setStyle(titleStyle);
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = props.gameEvent.title.empty() ? "Talk" : props.gameEvent.title;
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  // Create SectionScrollable for content area
  auto textSection = new SectionScrollable(window, this);
  textSection->setId("textSection");
  auto& textSectionStyle = textSection->getStyle();
  textSectionStyle.width = contentW;
  textSectionStyle.height =
      contentH - style.scale * BorderModalStandard::BOTTOM_BORDER_HEIGHT * style.scale -
      choiceHeightScaled;
  textSectionStyle.x = contentX;
  textSectionStyle.y = contentY;
  textSectionStyle.scale = style.scale;
  textSection->setProps(SectionScrollableProps{.scrollBarWidth = 40});
  children.push_back(std::unique_ptr<UiElement>(textSection));

  auto sepBorder = new OutsetRectangle(window, this);
  BaseStyle sepBorderStyle;
  sepBorderStyle.width = contentW;
  sepBorderStyle.height = 10;
  sepBorderStyle.x = contentX;
  sepBorderStyle.y = contentY + textSectionStyle.height;
  sepBorderStyle.scale = style.scale;
  sepBorder->setStyle(sepBorderStyle);
  sepBorder->setProps(OutsetRectangleProps{});
  children.push_back(std::unique_ptr<UiElement>(sepBorder));

  auto choiceSection = new SectionScrollable(window, this);
  choiceSection->setId("choiceSection");
  auto& choiceSectionStyle = choiceSection->getStyle();
  choiceSectionStyle.width = contentW;
  choiceSectionStyle.height = choiceHeightScaled;
  choiceSectionStyle.x = contentX;
  choiceSectionStyle.y = contentY + textSectionStyle.height + sepBorderStyle.height;
  choiceSectionStyle.scale = style.scale;
  choiceSection->setProps(SectionScrollableProps{.scrollBarWidth = 40});
  children.push_back(std::unique_ptr<UiElement>(choiceSection));

  // TODO: Add gameEvent rendering here
  // For now, the body section is set up but empty

  // Set SectionScrollable as content element of ModalStandard
  // modal->setContentElement(textSection);

  // Add modal to children
  // children.push_back(std::move(modal));
}

void PageTalkChoice::render(int dt) { UiElement::render(dt); }

} // namespace ui
