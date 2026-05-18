#include "MinipageEvent.h"
#include "ui/colors.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalSmall.h"
#include <memory>

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

  auto title = std::make_unique<TextLine>(window, modal.get());
  BaseStyle titleStyle;
  titleStyle.fontFamily = FontFamily::H2;
  titleStyle.fontSize = sdl2w::TEXT_SIZE_20;
  titleStyle.fontColor = Colors::Black;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  title->setStyle(titleStyle);

  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = "Event";
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  children.push_back(std::move(modal));
}

void MinipageEvent::render(int dt) { UiElement::render(dt); }

} // namespace ui
