#include "MinipageCharacterSheet.h"
#include "ui/colors.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalSmall.h"

namespace ui {

MinipageCharacterSheet::MinipageCharacterSheet(sdl2w::Window* _window,
                                               UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipageCharacterSheet::setProps(const MinipageCharacterSheetProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipageCharacterSheetProps& MinipageCharacterSheet::getProps() { return props; }

const MinipageCharacterSheetProps& MinipageCharacterSheet::getProps() const {
  return props;
}

const std::pair<int, int> MinipageCharacterSheet::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipageCharacterSheet::build() {
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

  auto title = bmin::makeUnique<TextLine>(window, modal.get());
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = "Character";
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  children.pushBack(bmin::UniquePtr<UiElement>(modal.release()));
}

void MinipageCharacterSheet::render(int dt) { UiElement::render(dt); }

} // namespace ui
