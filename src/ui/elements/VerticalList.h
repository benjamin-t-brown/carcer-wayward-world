#pragma once

#include "../UiElement.h"
#include "ui/elements/Quad.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "bmin/DynArray.h"

namespace ui {

struct VerticalListProps {
  int lineHeight = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// VerticalList element - renders an opinionated list of UiElements
class VerticalList : public UiElement {
private:
  VerticalListProps props;
  int selectedIndex = -1;

public:
  VerticalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~VerticalList() override = default;

  void setProps(const VerticalListProps& _props);
  VerticalListProps& getProps();
  const VerticalListProps& getProps() const;

  void setSelectedIndex(int index);
  int getSelectedIndex() const;
  void clearSelection();

  void addListItem(UiElement* item);
  void addListItems(const bmin::DynArray<UiElement*>& items);
  void removeListItemAtIndex(size_t index);

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
