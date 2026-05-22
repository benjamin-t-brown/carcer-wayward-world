#include "PageMagicSetup.h"
#include "ui/colors.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include <memory>

namespace ui {

PageMagicSetup::PageMagicSetup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageMagicSetup::setProps(const PageMagicSetupProps& _props) {
  props = _props;
  build();
}

PageMagicSetupProps& PageMagicSetup::getProps() { return props; }

const PageMagicSetupProps& PageMagicSetup::getProps() const { return props; }

const std::pair<int, int> PageMagicSetup::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageMagicSetup::build() {
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
  titleBlock.text = "Magic";
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
  bodyBlock.text = "Magic page placeholder.";
  bodyProps.textBlocks.push_back(bodyBlock);
  bodyText->setProps(bodyProps);
  scrollableSection->addChild(bodyText.release());

  // modal->setContentElement(scrollableSection.release());
  children.push_back(std::move(modal));
}

void PageMagicSetup::render(int dt) { UiElement::render(dt); }

} // namespace ui
