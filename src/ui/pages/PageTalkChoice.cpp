#include "PageTalkChoice.h"
#include "ui/colors.h"
#include "ui/components/BorderModalStandard.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
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

  // TODO: Add validation for gameEvent if needed
  // if (props.gameEvent.id.empty()) {
  //   return;
  // }

  // Create ModalStandard layout
  auto modal = std::make_unique<ModalStandard>(window, this);
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

  auto contentDims = modal->getContentDims();

  // Create title element
  auto title = std::make_unique<TextLine>(window, modal.get());
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
  modal->setTitleElement(std::move(title));

  // Create subtitle element
  auto subtitle = std::make_unique<TextLine>(window, modal.get());
  BaseStyle subtitleStyle;
  subtitleStyle.fontFamily = FontFamily::PARAGRAPH;
  subtitleStyle.fontSize = sdl2w::TEXT_SIZE_16;
  subtitleStyle.fontColor = Colors::White;
  subtitleStyle.textAlign = TextAlign::LEFT_TOP;
  subtitle->setStyle(subtitleStyle);
  TextLineProps subtitleProps;
  TextBlock subtitleBlock;
  subtitleBlock.text = props.portraitName.empty() ? "Subtitle" : props.portraitName;
  subtitleProps.textBlocks.push_back(subtitleBlock);
  subtitle->setProps(subtitleProps);
  modal->setSubtitleElement(std::move(subtitle));

  // Create SectionScrollable for content area
  auto scrollableSection = std::make_unique<SectionScrollable>(window, modal.get());
  scrollableSection->setId("scrollableSection");
  BaseStyle scrollableStyle;
  scrollableStyle.width = contentDims.first;
  scrollableStyle.height = contentDims.second;
  scrollableStyle.scale = 1;
  scrollableSection->setStyle(scrollableStyle);

  SectionScrollableProps scrollableProps;
  scrollableSection->setProps(scrollableProps);

  // TODO: Add gameEvent rendering here
  // For now, the body section is set up but empty

  // Set SectionScrollable as content element of ModalStandard
  modal->setContentElement(std::move(scrollableSection));

  // Add modal to children
  children.push_back(std::move(modal));
}

void PageTalkChoice::render(int dt) {
  UiElement::render(dt);
}

} // namespace ui

