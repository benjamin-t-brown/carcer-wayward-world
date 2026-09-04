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

export module carcer.ui.ListChCompactInfoVertical;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.SdlPixels;
export import carcer.ui.UiElement;
export import carcer.ui.components.ChCompactInfo;
import sdl2w;
import carcer.ui.elements;
#include "macros.h"

export {

// --- from ui/components/lists/ListChCompactInfoVertical.h ---
// IWYU pragma: keep

namespace ui {

struct ListChCompactInfoVerticalProps {
  bmin::DynArray<ChCompactInfoProps> entries;
  int selectedIndex = 0;
  int lineGap = 0;
};

// ListChCompactInfoVertical - vertical list of ChCompactInfo rows.
class ListChCompactInfoVertical : public UiElement {
private:
  ListChCompactInfoVerticalProps props;

public:
  ListChCompactInfoVertical(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListChCompactInfoVertical() override = default;

  void setProps(const ListChCompactInfoVerticalProps& _props);
  const ListChCompactInfoVerticalProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ListChCompactInfoVertical::ListChCompactInfoVertical(sdl2w::Window* _window,
                                                     UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListChCompactInfoVertical::getDims() const {
  if (children.empty()) {
    return {style.width, 0};
  }

  return children[0]->getDims();
}

void ListChCompactInfoVertical::setProps(const ListChCompactInfoVerticalProps& _props) {
  props = _props;
  build();
}

const ListChCompactInfoVerticalProps& ListChCompactInfoVertical::getProps() const {
  return props;
}

void ListChCompactInfoVertical::build() {
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

  style.width = chCompactInfoScaledWidth / style.scale;
  style.height = (chCompactInfoScaledHeight + props.lineGap * style.scale) *
                 props.entries.size() / style.scale;

  auto list = new VerticalList(window, this);
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

  VerticalListProps listProps;
  listProps.width = chCompactInfoScaledWidth;
  listProps.lineHeight = chCompactInfoScaledHeight;
  listProps.lineGap = static_cast<int>(props.lineGap * style.scale);
  list->setProps(listProps);

  addChild(list);
}

void ListChCompactInfoVertical::render(int dt) { UiElement::render(dt); }

} // namespace ui
