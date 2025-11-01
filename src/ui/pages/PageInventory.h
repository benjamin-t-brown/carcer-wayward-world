#pragma once

#include "ui/UiElement.h"
#include <string>

namespace ui {

// PageInventory-specific properties
struct PageInventoryProps {
  std::string characterPlayerId;
};

// PageInventory - renders the inventory page with ModalStandard layout containing
// a scrollable list of inventory items Uses Position, Size, Scale from BaseStyle
class PageInventory : public UiElement {
private:
  PageInventoryProps props;

public:
  PageInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageInventory() override = default;

  // Setters and getters for page-specific properties
  void setProps(const PageInventoryProps& _props);
  PageInventoryProps& getProps();
  const PageInventoryProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

