module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <algorithm>

export module carcer.ui.elements:ButtonGroup;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
export import carcer.ui.colors;
import sdl2w;
import :ButtonModal;
import :ButtonSprite;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonGroup.h ---
namespace ui {

enum class ButtonGroupAlignment { LEFT, CENTER, RIGHT };
enum class ButtonGroupButtonType { MODAL, SPRITE };

struct ButtonGroupButtonProps {
  bmin::String label;
  ButtonGroupButtonType type = ButtonGroupButtonType::MODAL;

  bmin::String spriteName;
  int spriteWidth = 16;
  int spriteHeight = 16;
  int spritePadding = 2;
  bool isSelected = false;
};
struct ButtonGroupProps {
  int width = 0;
  ButtonGroupAlignment alignment = ButtonGroupAlignment::LEFT;
  int buttonWidth = 80;
  int buttonHeight = 32;
  int buttonSpacing = 8; // Spacing between buttons
  int padding = 2;       // Inset around buttons; included in group width/height
  bmin::DynArray<ButtonGroupButtonProps> buttons;

  SDL_Color spriteBgColor = Colors::ButtonModalGrey1;
  SDL_Color spriteBgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color spriteBgColorBottomLeft = Colors::ButtonModalGrey3;
  int spriteBorderSize = 2;

  SDL_Color spriteSelectedBgColor = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorBottomLeft = Colors::ButtonModalSelected;
  int spriteSelectedBorderSize = 2;
};

class ButtonGroup : public UiElement {
private:
  ButtonGroupProps props;

public:
  ButtonGroup(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonGroup() override = default;

  void setProps(const ButtonGroupProps& _props);
  ButtonGroupProps& getProps();
  const ButtonGroupProps& getProps() const;

  void addObserverToButtonAtIndex(int index, UiEventObserver* observer);

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ButtonGroup::ButtonGroup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ButtonGroup::setProps(const ButtonGroupProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
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

  if (props.width > 0) {
    style.width = props.width;
  }

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
      button->setPos(buttonX, buttonY);
      button->setScale(style.scale);
      button->setProps(ButtonModalProps{
          .text = buttonProps.label,
          .width = props.buttonWidth,
          .height = props.buttonHeight,
      });
      addChild(button);
      break;
    }
    case ButtonGroupButtonType::SPRITE: {
      auto button = new ButtonSprite(window, this);
      button->setId("buttonGroupButton_" + bmin::toString(i));
      button->setPos(buttonX, buttonY);
      button->setScale(style.scale);
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
