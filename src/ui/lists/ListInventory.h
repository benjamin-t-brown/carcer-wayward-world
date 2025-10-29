#pragma once

#include "db/Database.h"
#include "types/Items.h"
#include "ui/UiElement.h"
#include <vector>

namespace ui {

struct ListInventoryProps {
  std::vector<std::string> itemNames;
};

// ListInventory - renders a vertical list of inventory items
class ListInventory : public UiElement {
private:
  ListInventoryProps props;
  db::Database* database = nullptr;

public:
  ListInventory(sdl2w::Window* _window,
                UiElement* _parent = nullptr,
                db::Database* _database = nullptr);
  ~ListInventory() override = default;

  void setProps(const ListInventoryProps& props);
  const ListInventoryProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

struct ListInventoryItemProps {
  const types::ItemTemplate* itemTemplate = nullptr;
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
