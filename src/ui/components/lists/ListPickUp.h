#pragma once

#include "ui/UiElement.h"
#include <string>
#include <vector>

namespace ui {

struct ListPickUpPropsItem {
  model::ItemInstance item;
  std::string itemLabel;
  int weight = 1;
  std::string itemSprite;
};

struct ListPickUpProps {
  std::vector<ListPickUpPropsItem> items;
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
