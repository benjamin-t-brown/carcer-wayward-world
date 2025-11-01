#include "PagePickUp.h"
#include "ui/colors.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/lists/ListPickUp.h"

namespace ui {

PagePickUp::PagePickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PagePickUp::setProps(const PagePickUpProps& _props) {
  props = _props;
  build();
}

PagePickUpProps& PagePickUp::getProps() { return props; }

const PagePickUpProps& PagePickUp::getProps() const { return props; }

const std::pair<int, int> PagePickUp::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PagePickUp::build() {
  children.clear();

  if (props.itemNames.empty()) {
    return;
  }

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
  titleBlock.text = "Pick Up";
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
  subtitleBlock.text = "Items";
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

  // Create ListPickUp inside the scrollable section
  auto listPickUp = std::make_unique<ListPickUp>(window, scrollableSection.get());
  listPickUp->setId("listPickUp");
  BaseStyle listStyle;
  listStyle.x = 0; // Positioned relative to scrollable section
  listStyle.y = 4;
  listStyle.width = contentDims.first - scrollableProps.scrollBarWidth - 8;
  listStyle.scale = 1;
  listPickUp->setStyle(listStyle);

  ListPickUpProps listProps;
  listProps.itemNames = props.itemNames;
  listPickUp->setProps(listProps);

  // Add ListPickUp to SectionScrollable
  scrollableSection->addChild(std::move(listPickUp));

  // Set SectionScrollable as content element of ModalStandard
  modal->setContentElement(std::move(scrollableSection));

  // Add modal to children
  children.push_back(std::move(modal));
}

void PagePickUp::render(int dt) {
  UiElement::render(dt);
}

} // namespace ui

