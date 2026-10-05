#pragma once

#include "ui/UiElement.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace ui {

struct ListSpecialItemsPropsItem {
  bmin::String itemName;
  bmin::String itemLabel;
  bmin::String itemSprite;
  int quantity = 1;
};

struct ListSpecialItemsProps {
  bmin::DynArray<ListSpecialItemsPropsItem> items;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

class ListSpecialItems : public UiElement {
private:
  ListSpecialItemsProps props;

  static constexpr int iconSpriteSize = 16;
  static constexpr float iconScale = 2.f;
  static constexpr int indexColumnWidth = 28;
  static constexpr int indexPaddingLeft = 4;
  static constexpr int reorderBtnHeight = 28;
  static constexpr int reorderBtnWidth = 14;
  static constexpr int reorderBtnGap = 2;
  static constexpr int reorderColumnGap = 2;
  static constexpr int contextBtnSize = 32;

  UiElement* createItemElement(const ListSpecialItemsPropsItem& item, int index);

public:
  ListSpecialItems(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListSpecialItems() override = default;

  void setProps(const ListSpecialItemsProps& _props);
  const ListSpecialItemsProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
