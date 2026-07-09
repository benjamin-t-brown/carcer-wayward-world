#pragma once

#include "../../UiElement.h"
#include "ui/colors.h"

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
