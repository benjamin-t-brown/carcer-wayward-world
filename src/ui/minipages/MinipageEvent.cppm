module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.minipages.MinipageEvent;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.core;
import carcer.ui.elements;
import carcer.ui.helpers;
import carcer.ui.layouts.ModalSmall;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/minipages/MinipageEvent.h ---
namespace ui {

struct MinipageEventProps {
  int width = 0;
  int height = 0;
};

// MinipageEvent - renders a small event shell using ModalSmall.
class MinipageEvent : public UiElement {
private:
  MinipageEventProps props;

public:
  MinipageEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageEvent() override = default;

  void setProps(const MinipageEventProps& _props);
  MinipageEventProps& getProps();
  const MinipageEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

MinipageEvent::MinipageEvent(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipageEvent::setProps(const MinipageEventProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = bmin::makeUnique<ModalSmall>(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);

  auto title = bmin::makeUnique<TextLine>(window, modal.get());
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = "Event";
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  children.pushBack(bmin::UniquePtr<UiElement>(modal.release()));
}

void MinipageEvent::render(int dt) { UiElement::render(dt); }

} // namespace ui
