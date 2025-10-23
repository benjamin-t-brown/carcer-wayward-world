#include "ButtonGroup.h"
#include "lib/sdl2w/Logger.h"
#include "ui/elements/ButtonModal.h"

namespace ui {

ButtonGroup::ButtonGroup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ButtonGroup::setProps(const ButtonGroupProps& _props) {
  props = _props;
  build();
}

ButtonGroupProps& ButtonGroup::getProps() { return props; }

const ButtonGroupProps& ButtonGroup::getProps() const { return props; }

void ButtonGroup::addObserverToButtonAtIndex(int index,
                                             std::unique_ptr<UiEventObserver> observer) {
  if (index >= 0 && index < static_cast<int>(children.size())) {
    children[index]->addEventObserver(std::move(observer));
  } else {
    LOG(ERROR) << "ButtonGroup: Index out of bounds when adding observer: " << index
               << LOG_ENDL;
  }
}

void ButtonGroup::build() {
  children.clear();
  int totalWidth = (props.buttonWidth + props.buttonSpacing) * props.buttonLabels.size();
  int numButtons = static_cast<int>(props.buttonLabels.size());

  for (const auto& buttonLabel : props.buttonLabels) {
    auto button = std::make_unique<ui::ButtonModal>(window);
    button->setId(buttonLabel);

    int currentX = 0;
    switch (props.alignment) {
    case ButtonGroupAlignment::LEFT:
      currentX = style.x + (numButtons / 2 * (props.buttonWidth + props.buttonSpacing));
      break;

    case ButtonGroupAlignment::CENTER:
      currentX = style.x - (totalWidth / 2) -
                 (numButtons / 2 * (props.buttonWidth + props.buttonSpacing));
      break;

    case ButtonGroupAlignment::RIGHT:
      currentX =
          style.x - totalWidth - (numButtons * (props.buttonWidth + props.buttonSpacing));
      break;
    }

    ui::BaseStyle buttonStyle;
    buttonStyle.x = currentX;
    buttonStyle.y = style.y;
    buttonStyle.width = props.buttonWidth;
    buttonStyle.height = props.buttonHeight;
    button->setStyle(buttonStyle);
    ui::ButtonModalProps buttonProps;
    buttonProps.text = buttonLabel;
    button->setProps(buttonProps);
    children.push_back(std::move(button));
  }
}

void ButtonGroup::render() { UiElement::render(); }

} // namespace ui
