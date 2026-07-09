#pragma once

#include "../UiElement.h"
#include "bmin/String.h"

namespace ui {

struct ItemInfoProps {
  bmin::String description;
  int weight = 0;
  int value = 0;
};

// ItemInfo - description, weight, and value for an item template.
class ItemInfo : public UiElement {
private:
  ItemInfoProps props;

  static constexpr int kVertSpacerHeight = 12;

public:
  ItemInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ItemInfo() override = default;

  void setProps(const ItemInfoProps& _props);
  const ItemInfoProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
