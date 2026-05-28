#pragma once

#include "ui/UiElement.h"

namespace ui {

struct ListInventoryPropsItem {
  std::string itemId;
  std::string itemName;
  std::string itemLabel;
  std::string itemSprite;
  bool isEquippable = false;
  bool isEquipped = false;
  bool isStackable = false;
  int quantity = 1;
};
struct ListInventoryProps {
  std::string characterPlayerId;
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
  const int indexColumnWidth = 28;
  const int indexPaddingLeft = 4;
  const int reorderBtnHeight = 28;
  const int reorderBtnWidth = 14;
  const int reorderBtnGap = 2;
  const int reorderColumnGap = 2;

  UiElement* createItemElement(const ListInventoryPropsItem& item, int index);

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
