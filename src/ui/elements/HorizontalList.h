#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep

namespace ui {

struct HorizontalListProps {
  int lineWidth = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// HorizontalList element - lays out child UiElements in a row.
class HorizontalList : public UiElement {
private:
  HorizontalListProps props;
  int selectedIndex = -1;

public:
  HorizontalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~HorizontalList() override = default;

  void setProps(const HorizontalListProps& _props);
  HorizontalListProps& getProps();
  const HorizontalListProps& getProps() const;

  void setSelectedIndex(int index);
  int getSelectedIndex() const;
  void clearSelection();

  void addListItem(UiElement* item);
  void addListItems(const DynArray<UiElement*>& items);
  void removeListItemAtIndex(size_t index);

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
