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

  auto title = new TextLine(window, this);
  title->setId("title");
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.x = style.x + paddingScaled;
  titleStyle.y = style.y + paddingScaled;
  titleStyle.scale = 1.f;
  titleStyle.fontColor = Colors::Black;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps titleProps;
  titleProps.textBlocks.pushBack({.text = props.title});
  title->setProps(titleProps);
  auto [titleWidth, titleHeight] = title->calculateTextDims();
  addChild(title);

  const int messageY = style.y + paddingScaled + titleHeight + paddingScaled;

  auto message = new TextParagraph(window, this);
  message->setId("message");
  auto& messageStyle = message->getStyle();
  messageStyle.x = style.x + paddingScaled;
  messageStyle.y = messageY;
  messageStyle.width = contentWidth;
  messageStyle.scale = 1.f;
  setBaseFontConfig(messageStyle, BaseFontConfig::MODAL_TEXT);
  messageStyle.fontColor = Colors::Black;
  messageStyle.textAlign = TextAlign::LEFT_TOP;
  messageStyle.lineSpacing = 0;
  TextParagraphProps messageProps;
  messageProps.textBlocks.pushBack({.text = props.message});
  message->setProps(messageProps);
  auto [_, messageHeight] = message->getDims();
  addChild(message);

  const int buttonsY = messageY + messageHeight + paddingScaled;

  buttonGroup = new ButtonGroup(window, this);
  buttonGroup->setId("buttonGroup");
  auto& groupStyle = buttonGroup->getStyle();
  groupStyle.x = style.x + paddingScaled;
  groupStyle.y = buttonsY;
  groupStyle.width = contentWidth;
  groupStyle.scale = style.scale;
  ButtonGroupProps groupProps;
  groupProps.alignment = ButtonGroupAlignment::RIGHT;
  groupProps.buttons.pushBack({.label = props.cancelButtonLabel});
  groupProps.buttons.pushBack({.label = props.confirmButtonLabel});
  buttonGroup->setProps(groupProps);
  auto [__, buttonGroupHeight] = buttonGroup->getDims();
  addChild(buttonGroup);

  style.height = ((buttonsY + buttonGroupHeight + paddingScaled) - style.y) / style.scale;

  auto border = new BorderDropShadow(window, this);
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  borderStyle.scale = style.scale;
  border->setProps(BorderDropShadowProps{});
  children.insert(children.begin(), bmin::UniquePtr<UiElement>(border));
}

void ConfirmModal::render(int dt) { UiElement::render(dt); }

} // namespace ui
