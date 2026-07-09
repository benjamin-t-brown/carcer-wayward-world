#pragma once

#include "ui/UiElement.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace ui {

struct ListPickUpPropsItem {
  model::ItemInstance item;
  bmin::String itemLabel;
  int weight = 1;
  bmin::String itemSprite;
};

struct ListPickUpProps {
  bmin::DynArray<ListPickUpPropsItem> items;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

// ListPickUp - renders a vertical list of items that can be picked up
class ListPickUp : public UiElement {
private:
  ListPickUpProps props;

  const int contextBtnSize = 32;
  const int iconSpriteSize = 16;

  UiElement* createItemElement(const ListPickUpPropsItem& item);

public:
  ListPickUp(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListPickUp() override = default;

  void setProps(const ListPickUpProps& props);
  const ListPickUpProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
