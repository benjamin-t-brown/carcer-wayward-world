#include "PageCharacter.h"
#include "ui/colors.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include <memory>

namespace ui {

PageCharacter::PageCharacter(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageCharacter::setProps(const PageCharacterProps& _props) {
  props = _props;
  build();
}

PageCharacterProps& PageCharacter::getProps() { return props; }

const PageCharacterProps& PageCharacter::getProps() const { return props; }

const std::pair<int, int> PageCharacter::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageCharacter::build() {
  children.clear();

  auto modal = std::make_unique<ModalStandard>(window, this);
  modal->setId("modal");
  BaseStyle modalStyle;
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setStyle(modalStyle);
  modal->setProps(ModalStandardProps{});

  auto [contentW, contentH] = modal->getContentDims();

  auto title = std::make_unique<TextLine>(window, modal.get());
  BaseStyle titleStyle;
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  title->setStyle(titleStyle);

  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = "Character";
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  auto scrollableSection = std::make_unique<SectionScrollable>(window, modal.get());
  scrollableSection->setId("scrollableSection");
  BaseStyle scrollableStyle;
  scrollableStyle.width = contentW;
  scrollableStyle.height = contentH;
  scrollableStyle.scale = style.scale;
  scrollableSection->setStyle(scrollableStyle);
  scrollableSection->setProps(SectionScrollableProps{});

  auto bodyText = std::make_unique<TextLine>(window, scrollableSection.get());
  bodyText->setId("bodyText");
  BaseStyle bodyStyle;
  bodyStyle.x = 0;
  bodyStyle.y = 4;
  setBaseFontConfig(bodyStyle, BaseFontConfig::MODAL_TEXT);
  bodyStyle.textAlign = TextAlign::LEFT_TOP;
  bodyStyle.scale = style.scale;
  bodyText->setStyle(bodyStyle);

  TextLineProps bodyProps;
  TextBlock bodyBlock;
  bodyBlock.text = "Character page placeholder.";
  bodyProps.textBlocks.push_back(bodyBlock);
  bodyText->setProps(bodyProps);
  scrollableSection->addChild(bodyText.release());

  // modal->setContentElement(scrollableSection.release());
  children.push_back(std::move(modal));
}

void PageCharacter::render(int dt) { UiElement::render(dt); }

} // namespace ui
