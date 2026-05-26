#pragma once

#include "../../UiElement.h"

namespace ui {

enum class ButtonGroupAlignment { LEFT, CENTER, RIGHT };
enum class ButtonGroupButtonType { MODAL };

struct ButtonGroupButtonProps {
  std::string label;
  ButtonGroupButtonType type = ButtonGroupButtonType::MODAL;
};
struct ButtonGroupProps {
  ButtonGroupAlignment alignment = ButtonGroupAlignment::LEFT;
  int buttonWidth = 100;
  int buttonHeight = 50;
  int buttonSpacing = 8; // Spacing between buttons
  int padding = 2;       // Inset around buttons; included in group width/height
  std::vector<ButtonGroupButtonProps> buttons;
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
