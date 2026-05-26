#include "ButtonGroup.h"
#include "ButtonModal.h"
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

  auto [scaledWidth, scaledHeight] = getDims();

  for (int i = 0; i < static_cast<int>(props.buttons.size()); i++) {
    const auto& buttonProps = props.buttons[i];
    switch (buttonProps.type) {
    case ButtonGroupButtonType::MODAL:
      auto button = new ButtonModal(window, this);
      button->setId("buttonGroupButton_" + std::to_string(i));
      auto& buttonStyle = button->getStyle();
      buttonStyle.width = props.buttonWidth;
      buttonStyle.height = props.buttonHeight;
      buttonStyle.scale = style.scale;
      button->setProps(ButtonModalProps{buttonProps.label});
      addChild(button);

      auto scaledPadding = static_cast<int>(props.padding * style.scale);
      auto scaledButtonWidth = static_cast<int>(props.buttonWidth * style.scale);
      auto scaledButtonSpacing = static_cast<int>(props.buttonSpacing * style.scale);

      buttonStyle.y = style.y + scaledPadding;
      switch (props.alignment) {
      case ButtonGroupAlignment::LEFT:
        buttonStyle.x =
            style.x + scaledPadding + i * (scaledButtonWidth + scaledButtonSpacing);
        break;
      case ButtonGroupAlignment::CENTER:
        buttonStyle.x = style.x + scaledWidth / 2 - scaledButtonWidth / 2 -
                        i * (scaledButtonWidth + scaledButtonSpacing);
        break;
      case ButtonGroupAlignment::RIGHT:
        buttonStyle.x = style.x + scaledWidth - scaledPadding -
                        (i + 1) * (scaledButtonWidth + ( i > 0 ? scaledButtonSpacing : 0));
        break;
      }

      button->build();
      break;
    }
  }
}

void ButtonGroup::render(int dt) { UiElement::render(dt); }

} // namespace ui
