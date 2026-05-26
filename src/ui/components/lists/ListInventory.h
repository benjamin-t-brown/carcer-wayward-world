#pragma once

#include "ui/UiElement.h"

namespace ui {

struct ListInventoryPropsItem {
  std::string itemId;
  std::string itemName;
  std::string itemLabel;
  std::string itemSprite;
};
struct ListInventoryProps {
  std::vector<ListInventoryPropsItem> items;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

// ListInventory - renders a vertical list of inventory items
class ListInventory : public UiElement {
private:
  ListInventoryProps props;

  const int contextBtnSize = 32;
  const int iconSpriteSize = 16;

  UiElement* createItemElement(const ListInventoryPropsItem& item);

public:
  ListInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListInventory() override = default;

  void setProps(const ListInventoryProps& props);
  const ListInventoryProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
