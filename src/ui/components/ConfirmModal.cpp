#include "ConfirmModal.h"
#include "ui/colors.h"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonGroup.h"

namespace ui {

ConfirmModal::ConfirmModal(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  style.width = 320;
}

void ConfirmModal::setProps(const ConfirmModalProps& _props) {
  props = _props;
  build();
}

ConfirmModalProps& ConfirmModal::getProps() { return props; }

const ConfirmModalProps& ConfirmModal::getProps() const { return props; }

ButtonGroup* ConfirmModal::getButtonGroup() { return buttonGroup; }

const ButtonGroup* ConfirmModal::getButtonGroup() const { return buttonGroup; }

void ConfirmModal::build() {
  children.clear();
  buttonGroup = nullptr;

  const int padding = 8;
  const int paddingScaled = static_cast<int>(padding * style.scale);
  const int contentWidth = style.width - 2 * padding;

  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);

  auto title = new TextLine(window, this);
  title->setId("title");
  title->setPos(style.x + paddingScaled, style.y + paddingScaled);
  title->setScale(1.f);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  titleProps.textBlocks.pushBack({.text = props.title});
  title->setProps(titleProps);
  auto [titleWidth, titleHeight] = title->calculateTextDims();
  addChild(title);

  const int messageY = style.y + paddingScaled + titleHeight + paddingScaled;

  TextFontProps messageFont;
  setBaseFontConfig(messageFont, BaseFontConfig::MODAL_TEXT);

  auto message = new TextParagraph(window, this);
  message->setId("message");
  message->setPos(style.x + paddingScaled, messageY);
  message->setScale(1.f);
  TextParagraphProps messageProps;
  messageProps.width = contentWidth;
  messageProps.fontFamily = messageFont.fontFamily;
  messageProps.fontSize = messageFont.fontSize;
  messageProps.fontColor = Colors::Black;
  messageProps.textAlign = TextAlign::LEFT_TOP;
  messageProps.lineSpacing = 0;
  messageProps.textBlocks.pushBack({.text = props.message});
  message->setProps(messageProps);
  auto [_, messageHeight] = message->getDims();
  addChild(message);

  const int buttonsY = messageY + messageHeight + paddingScaled;

  buttonGroup = new ButtonGroup(window, this);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(style.x + paddingScaled, buttonsY);
  buttonGroup->setScale(style.scale);
  ButtonGroupProps groupProps;
  groupProps.alignment = ButtonGroupAlignment::RIGHT;
  groupProps.buttons.pushBack({.label = props.cancelButtonLabel});
  groupProps.buttons.pushBack({.label = props.confirmButtonLabel});
  buttonGroup->setProps(groupProps);
  auto [__, buttonGroupHeight] = buttonGroup->getDims();
  addChild(buttonGroup);

  style.height = ((buttonsY + buttonGroupHeight + paddingScaled) - style.y) / style.scale;

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  children.insert(children.begin(), bmin::UniquePtr<UiElement>(border));
}

void ConfirmModal::render(int dt) { UiElement::render(dt); }

} // namespace ui
