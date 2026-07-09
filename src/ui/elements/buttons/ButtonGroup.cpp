#include "ButtonGroup.h"
#include "ButtonModal.h"
#include "ButtonSprite.h"
#include "lib/sdl2w/Logger.h"
#include <algorithm>

namespace ui {

ButtonGroup::ButtonGroup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ButtonGroup::setProps(const ButtonGroupProps& _props) {
  props = _props;
  build();
}

ButtonGroupProps& ButtonGroup::getProps() { return props; }

const ButtonGroupProps& ButtonGroup::getProps() const { return props; }

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
  const int contentLogicalWidth = props.padding * 2 + totalButtonWidth;
  style.width = std::max(style.width, contentLogicalWidth);
  style.height = props.padding * 2 + props.buttonHeight;

  const int alignmentScaledWidth = static_cast<int>(style.width * style.scale);

  const auto scaledPadding = static_cast<int>(props.padding * style.scale);
  const auto scaledButtonWidth = static_cast<int>(props.buttonWidth * style.scale);
  const auto scaledButtonSpacing = static_cast<int>(props.buttonSpacing * style.scale);
  const int rowWidth =
      count * scaledButtonWidth + (count - 1) * scaledButtonSpacing;

  for (int i = 0; i < count; ++i) {
    const auto& buttonProps = props.buttons[static_cast<size_t>(i)];

    int buttonX = 0;
    switch (props.alignment) {
    case ButtonGroupAlignment::LEFT:
      buttonX = style.x + scaledPadding + i * (scaledButtonWidth + scaledButtonSpacing);
      break;
    case ButtonGroupAlignment::CENTER:
      buttonX = style.x + (alignmentScaledWidth - rowWidth) / 2 +
                i * (scaledButtonWidth + scaledButtonSpacing);
      break;
    case ButtonGroupAlignment::RIGHT:
      buttonX = style.x + alignmentScaledWidth - scaledPadding -
                (i + 1) * scaledButtonWidth - i * scaledButtonSpacing;
      break;
    }

    const int buttonY = style.y + scaledPadding;

    switch (buttonProps.type) {
    case ButtonGroupButtonType::MODAL: {
      auto button = new ButtonModal(window, this);
      button->setId("buttonGroupButton_" + bmin::toString(i));
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
      button->setId("buttonGroupButton_" + bmin::toString(i));
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
