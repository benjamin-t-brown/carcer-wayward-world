#include "MinipageEvent.h"
#include "ui/colors.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalSmall.h"

namespace ui {

MinipageEvent::MinipageEvent(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipageEvent::setProps(const MinipageEventProps& _props) {
  props = _props;
  build();
}

MinipageEventProps& MinipageEvent::getProps() { return props; }

const MinipageEventProps& MinipageEvent::getProps() const { return props; }

const std::pair<int, int> MinipageEvent::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipageEvent::build() {
  children.clear();

  auto modal = bmin::makeUnique<ModalSmall>(window, this);
  modal->setId("modal");
  BaseStyle modalStyle;
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setStyle(modalStyle);
  modal->setProps(ModalSmallProps{});

  auto title = bmin::makeUnique<TextLine>(window, modal.get());
  BaseStyle titleStyle;
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.fontColor = Colors::Black;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  title->setStyle(titleStyle);

  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = "Event";
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  children.pushBack(bmin::UniquePtr<UiElement>(modal.release()));
}

void MinipageEvent::render(int dt) { UiElement::render(dt); }

} // namespace ui
