#pragma once

#include "ui/UiElement.h"
#include <string>

namespace ui {

struct PopupPickupItemProps {
  std::string spriteName;
  std::string label;
  std::string description;
  int weight;
  int value;
};

// PopupPickupItem - shows info about an item that can be picked up
// Uses Position, Size, Scale from BaseStyle
class PopupPickupItem : public UiElement {
private:
  PopupPickupItemProps props;
  layers::Layer* layer;

public:
  PopupPickupItem(sdl2w::Window* _window, layers::Layer* _layer);

  // Setters and getters for popup-specific properties
  void setProps(const PopupPickupItemProps& _props);
  PopupPickupItemProps& getProps();
  const PopupPickupItemProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
