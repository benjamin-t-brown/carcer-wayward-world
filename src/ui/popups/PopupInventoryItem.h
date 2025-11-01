#pragma once

#include "ui/UiElement.h"
#include <string>

namespace ui {

// PopupInventoryItem-specific properties
struct PopupInventoryItemProps {
  std::string itemName;
  std::string itemId;
};

// PopupInventoryItem - shows info about an item and actions that can be done from
// inventory Uses Position, Size, Scale from BaseStyle
class PopupInventoryItem : public UiElement {
private:
  PopupInventoryItemProps props;
  layers::Layer* layer;

public:
  PopupInventoryItem(sdl2w::Window* _window, layers::Layer* _layer);

  // Setters and getters for popup-specific properties
  void setProps(const PopupInventoryItemProps& _props);
  PopupInventoryItemProps& getProps();
  const PopupInventoryItemProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
