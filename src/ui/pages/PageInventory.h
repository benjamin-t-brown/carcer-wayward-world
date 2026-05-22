#pragma once

#include "model/Character.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListInventory.h"

namespace ui {

struct PageInventoryProps {
  model::CharacterPlayer* characterPlayer = nullptr;
};

class PageInventory : public UiElement, public state::DatabaseInterface {
private:
  PageInventoryProps props;

  void populateInventoryProps(std::vector<ListInventoryPropsItem>& listProps);

public:
  PageInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageInventory() override = default;

  // Setters and getters for page-specific properties
  void setProps(const PageInventoryProps& _props);
  PageInventoryProps& getProps();
  const PageInventoryProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
