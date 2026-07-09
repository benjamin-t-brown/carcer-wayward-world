#include "PageMagicSetup.h"
#include "ui/colors.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"

namespace ui {

PageMagicSetup::PageMagicSetup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageMagicSetup::setProps(const PageMagicSetupProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = bmin::makeUnique<ModalStandard>(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalStandardProps{
      .width = style.width,
      .height = style.height,
  });

  auto [contentW, contentH] = modal->getContentDims();

  auto title = bmin::makeUnique<TextLine>(window, modal.get());
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = "Magic";
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  auto scrollableSection = bmin::makeUnique<SectionScrollable>(window, modal.get());
  scrollableSection->setId("scrollableSection");
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = contentW,
      .height = contentH,
  });

  auto bodyText = bmin::makeUnique<TextLine>(window, scrollableSection.get());
  bodyText->setId("bodyText");
  TextFontProps bodyFont;
  setBaseFontConfig(bodyFont, BaseFontConfig::MODAL_TEXT);
  bodyText->setPos(0, 4);
  bodyText->setScale(style.scale);
  TextLineProps bodyProps;
  bodyProps.fontFamily = bodyFont.fontFamily;
  bodyProps.fontSize = bodyFont.fontSize;
  bodyProps.fontColor = bodyFont.fontColor;
  bodyProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock bodyBlock;
  bodyBlock.text = "Magic page placeholder.";
  bodyProps.textBlocks.pushBack(bodyBlock);
  bodyText->setProps(bodyProps);
  scrollableSection->addChild(bodyText.release());

  // modal->setContentElement(scrollableSection.release());
  children.pushBack(bmin::UniquePtr<UiElement>(modal.release()));
}

void PageMagicSetup::render(int dt) { UiElement::render(dt); }

} // namespace ui
