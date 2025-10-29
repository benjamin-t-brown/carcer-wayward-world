#pragma once

#include "../UiElement.h"

namespace ui {

// Alignment enum for button groups
enum class ButtonGroupAlignment { LEFT, CENTER, RIGHT };

// ButtonGroup-specific properties
struct ButtonGroupProps {
  ButtonGroupAlignment alignment = ButtonGroupAlignment::LEFT;
  int buttonWidth = 100;
  int buttonHeight = 50;
  int buttonSpacing = 5; // Spacing between buttons
  std::vector<std::string> buttonLabels;
};

// ButtonGroup element - groups buttons horizontally with different alignment options
// Uses Position from BaseStyle (Width and Height are ignored)
class ButtonGroup : public UiElement {
private:
  ButtonGroupProps props;

public:
  ButtonGroup(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonGroup() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonGroupProps& _props);
  ButtonGroupProps& getProps();
  const ButtonGroupProps& getProps() const;

  void addObserverToButtonAtIndex(int index, std::unique_ptr<UiEventObserver> observer);

  void build() override;
  void render(int dt) override;
};

} // namespace ui
