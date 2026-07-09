#pragma once

#include "../../UiElement.h"
#include "lib/Types.h"

namespace ui {

struct ButtonIconProps {
  String regularSprite;
  String activeSprite;
  int iconSize = 32;
  bool isDisabled = false;
};

// ButtonIcon - clickable button that displays a regular or active sprite.
class ButtonIcon : public UiElement {
private:
  ButtonIconProps props;
  bool isInActiveMode = false;

public:
  static inline constexpr const char* MINUS_ICON1 = "ui_icon_buttons_0";
  static inline constexpr const char* MINUS_ICON2 = "ui_icon_buttons_8";
  static inline constexpr const char* PLUS_ICON1 = "ui_icon_buttons_1";
  static inline constexpr const char* PLUS_ICON2 = "ui_icon_buttons_9";
  static inline constexpr const char* QUESTION_ICON1 = "ui_icon_buttons_2";
  static inline constexpr const char* QUESTION_ICON2 = "ui_icon_buttons_10";
  static inline constexpr const char* HAMBURGER_ICON1 = "ui_icon_buttons_3";
  static inline constexpr const char* HAMBURGER_ICON2 = "ui_icon_buttons_11";

  bool isActive = false;
  ButtonIcon(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonIcon() override = default;

  void setProps(const ButtonIconProps& _props);
  ButtonIconProps& getProps();
  const ButtonIconProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
