#include "MinipagePickUp.h"
#include "ui/colors.h"
#include "ui/components/BorderModalSmall.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalSmall.h"
#include "ui/lists/ListPickUp.h"

namespace ui {

MinipagePickUp::MinipagePickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipagePickUp::setProps(const MinipagePickUpProps& _props) {
  props = _props;
  build();
}

MinipagePickUpProps& MinipagePickUp::getProps() { return props; }

const MinipagePickUpProps& MinipagePickUp::getProps() const { return props; }

const std::pair<int, int> MinipagePickUp::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipagePickUp::build() {
  children.clear();

  if (props.itemNames.empty()) {
    return;
  }

  auto modal = std::make_unique<ModalSmall>(window, this);
  modal->setId("modal");
  BaseStyle modalStyle;
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setStyle(modalStyle);
  modal->setProps(ModalSmallProps{});

  auto borderElement = dynamic_cast<BorderModalSmall*>(modal->getChildById("border"));
  if (borderElement == nullptr) {
    return;
  }
  auto contentDims = borderElement->getContentDims();

  auto title = std::make_unique<TextLine>(window, modal.get());
  BaseStyle titleStyle;
  titleStyle.fontFamily = FontFamily::H2;
  titleStyle.fontSize = sdl2w::TEXT_SIZE_20;
  titleStyle.fontColor = Colors::Black;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  title->setStyle(titleStyle);
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = "Pick Up";
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  auto scrollableSection = std::make_unique<SectionScrollable>(window, modal.get());
  scrollableSection->setId("scrollableSection");
  BaseStyle scrollableStyle;
  scrollableStyle.width = contentDims.first;
  scrollableStyle.height = contentDims.second;
  scrollableStyle.scale = style.scale;
  scrollableSection->setStyle(scrollableStyle);

  SectionScrollableProps scrollableProps;
  scrollableSection->setProps(scrollableProps);

  auto listPickUp = std::make_unique<ListPickUp>(window, scrollableSection.get());
  listPickUp->setId("listPickUp");
  BaseStyle listStyle;
  listStyle.x = 0;
  listStyle.y = 4;
  listStyle.width = contentDims.first - scrollableProps.scrollBarWidth - 8;
  listStyle.scale = style.scale;
  listPickUp->setStyle(listStyle);

  ListPickUpProps listProps;
  listProps.itemNames = props.itemNames;
  listPickUp->setProps(listProps);

  scrollableSection->addChild(listPickUp.release());
  modal->setContentElement(scrollableSection.release());
  children.push_back(std::move(modal));
}

void MinipagePickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui
