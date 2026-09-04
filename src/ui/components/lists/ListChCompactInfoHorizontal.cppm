module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.lists:ListChCompactInfoHorizontal;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
export import carcer.ui.components;
import sdl2w;
import carcer.ui.elements;
#include "macros.h"

export {

// --- from ui/components/lists/ListChCompactInfoHorizontal.h ---
// IWYU pragma: keep

namespace ui {

struct ListChCompactInfoHorizontalProps {
  bmin::DynArray<ChCompactInfoProps> entries;
  int selectedIndex = 0;
  int lineGap = 0;
};

// ListChCompactInfoHorizontal - horizontal list of ChCompactInfo rows.
class ListChCompactInfoHorizontal : public UiElement {
private:
  ListChCompactInfoHorizontalProps props;

public:
  ListChCompactInfoHorizontal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListChCompactInfoHorizontal() override = default;

  void setProps(const ListChCompactInfoHorizontalProps& _props);
  const ListChCompactInfoHorizontalProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ListChCompactInfoHorizontal::ListChCompactInfoHorizontal(sdl2w::Window* _window,
                                                         UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListChCompactInfoHorizontal::getDims() const {
  if (children.empty()) {
    return {0, style.height};
  }

  return children[0]->getDims();
}

void ListChCompactInfoHorizontal::setProps(
    const ListChCompactInfoHorizontalProps& _props) {
  props = _props;
  build();
}

const ListChCompactInfoHorizontalProps& ListChCompactInfoHorizontal::getProps() const {
  return props;
}

void ListChCompactInfoHorizontal::build() {
  children.clear();

  if (props.entries.empty()) {
    return;
  }

  int numStatusColumns = 2;

  auto defaultChCompactInfo = ChCompactInfo(window, nullptr);
  defaultChCompactInfo.setScale(style.scale);
  defaultChCompactInfo.setProps(ChCompactInfoProps{
      .numStatusColumns = numStatusColumns,
  });
  auto [chCompactInfoScaledWidth, chCompactInfoScaledHeight] =
      defaultChCompactInfo.getDims();

  style.width = (chCompactInfoScaledWidth + props.lineGap * style.scale) *
                props.entries.size() / style.scale;
  style.height = (chCompactInfoScaledHeight + props.lineGap * style.scale) / style.scale;

  auto list = new HorizontalList(window, this);
  list->setId("list");
  list->setPos(style.x, style.y);
  list->setScale(1.f);

  for (size_t i = 0; i < props.entries.size(); i++) {
    auto chCompactInfo = new ChCompactInfo(window, this);
    chCompactInfo->setPos(style.x, style.y);
    chCompactInfo->setScale(style.scale);
    auto chCompactInfoProps = props.entries[i];
    chCompactInfoProps.numStatusColumns = numStatusColumns;
    chCompactInfoProps.isSelected = static_cast<int>(i) == props.selectedIndex;
    chCompactInfo->setProps(chCompactInfoProps);
    list->addChild(chCompactInfo);
  }

  HorizontalListProps listProps;
  listProps.height = chCompactInfoScaledHeight;
  listProps.lineWidth = chCompactInfoScaledWidth;
  listProps.lineGap = static_cast<int>(props.lineGap * style.scale);
  list->setProps(listProps);

  addChild(list);
}

void ListChCompactInfoHorizontal::render(int dt) { UiElement::render(dt); }

} // namespace ui
