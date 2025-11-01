#pragma once

#include "db/Database.h"
#include "model/Character.h"
#include "model/Items.h"
#include "ui/UiElement.h"

namespace ui {

struct ListInventoryProps {
  const model::CharacterPlayer* character = nullptr;
};

// ListInventory - renders a vertical list of inventory items
class ListInventory : public UiElement {
private:
  ListInventoryProps props;

public:
  ListInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListInventory() override = default;

  void setProps(const ListInventoryProps& props);
  const ListInventoryProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

struct ListInventoryItemProps {
  const model::ItemTemplate* itemTemplate = nullptr;
  std::string itemId;
  int quantity = 1;
};
// ListInventoryItem - renders a single inventory item with icon, label, and context
// button
class ListInventoryItem : public UiElement {
private:
  ListInventoryItemProps props;
  const SDL_Color bgColor = Colors::White;

public:
  ListInventoryItem(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListInventoryItem() override = default;

  // Set the item template data
  void setProps(const ListInventoryItemProps& props);
  const ListInventoryItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
