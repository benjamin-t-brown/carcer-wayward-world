#include "ButtonGroup.h"
#include "ButtonModal.h"
#include "ButtonSprite.h"
#include "lib/sdl2w/Logger.h"

namespace ui {

ButtonGroup::ButtonGroup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ButtonGroup::setProps(const ButtonGroupProps& _props) {
  props = _props;
  build();
}

ButtonGroupProps& ButtonGroup::getProps() { return props; }

const ButtonGroupProps& ButtonGroup::getProps() const { return props; }

const std::pair<int, int> ButtonGroup::getDims() const {
  if (props.buttons.empty()) {
    return {0, 0};
  }

  const int count = static_cast<int>(props.buttons.size());
  const int totalButtonWidth =
      count * props.buttonWidth + (count - 1) * props.buttonSpacing;
  const int logicalWidth = props.padding * 2 + totalButtonWidth;
  const int logicalHeight = props.padding * 2 + props.buttonHeight;

  return {static_cast<int>(logicalWidth * style.scale),
          static_cast<int>(logicalHeight * style.scale)};
}

void ButtonGroup::addObserverToButtonAtIndex(int index, UiEventObserver* observer) {
  if (index >= 0 && index < static_cast<int>(children.size())) {
    children[index]->addEventObserver(observer);
  } else {
    LOG(ERROR) << "ButtonGroup: Index out of bounds when adding observer: " << index
               << LOG_ENDL;
  }
}

void ButtonGroup::build() {
  children.clear();

  if (props.buttons.empty()) {
    return;
  }

  const int count = static_cast<int>(props.buttons.size());
  const int totalButtonWidth =
      count * props.buttonWidth + (count - 1) * props.buttonSpacing;
  style.width = props.padding * 2 + totalButtonWidth;
  style.height = props.padding * 2 + props.buttonHeight;

  auto [scaledWidth, scaledHeight] = getDims();
  (void)scaledHeight;

  const auto scaledPadding = static_cast<int>(props.padding * style.scale);
  const auto scaledButtonWidth = static_cast<int>(props.buttonWidth * style.scale);
  const auto scaledButtonSpacing = static_cast<int>(props.buttonSpacing * style.scale);

  for (int i = 0; i < count; ++i) {
    const auto& buttonProps = props.buttons[static_cast<size_t>(i)];

    int buttonX = 0;
    switch (props.alignment) {
    case ButtonGroupAlignment::LEFT:
      buttonX = style.x + scaledPadding + i * (scaledButtonWidth + scaledButtonSpacing);
      break;
    case ButtonGroupAlignment::CENTER:
      buttonX = style.x + scaledWidth / 2 - scaledButtonWidth / 2 -
                i * (scaledButtonWidth + scaledButtonSpacing);
      break;
    case ButtonGroupAlignment::RIGHT:
      buttonX = style.x + scaledWidth - scaledPadding -
                (i + 1) * scaledButtonWidth - i * scaledButtonSpacing;
      break;
    }

    const int buttonY = style.y + scaledPadding;

    switch (buttonProps.type) {
    case ButtonGroupButtonType::MODAL: {
      auto button = new ButtonModal(window, this);
      button->setId("buttonGroupButton_" + std::to_string(i));
      auto& buttonStyle = button->getStyle();
      buttonStyle.x = buttonX;
      buttonStyle.y = buttonY;
      buttonStyle.width = props.buttonWidth;
      buttonStyle.height = props.buttonHeight;
      buttonStyle.scale = style.scale;
      button->setProps(ButtonModalProps{buttonProps.label});
      addChild(button);
      button->build();
      break;
    }
    case ButtonGroupButtonType::SPRITE: {
      auto button = new ButtonSprite(window, this);
      button->setId("buttonGroupButton_" + std::to_string(i));
      auto& buttonStyle = button->getStyle();
      buttonStyle.x = buttonX;
      buttonStyle.y = buttonY;
      buttonStyle.scale = style.scale;
      button->setProps(ButtonSpriteProps{
          .spriteName = buttonProps.spriteName,
          .spriteWidth = buttonProps.spriteWidth,
          .spriteHeight = buttonProps.spriteHeight,
          .padding = buttonProps.spritePadding,
          .isSelected = buttonProps.isSelected,
          .bgColor = props.spriteBgColor,
          .bgColorTopRight = props.spriteBgColorTopRight,
          .bgColorBottomLeft = props.spriteBgColorBottomLeft,
          .borderSize = props.spriteBorderSize,
          .selectedBgColor = props.spriteSelectedBgColor,
          .selectedBgColorTopRight = props.spriteSelectedBgColorTopRight,
          .selectedBgColorBottomLeft = props.spriteSelectedBgColorBottomLeft,
          .selectedBorderSize = props.spriteSelectedBorderSize,
      });
      addChild(button);
      break;
    }
    }
  }
}

void ButtonGroup::render(int dt) { UiElement::render(dt); }

} // namespace ui
