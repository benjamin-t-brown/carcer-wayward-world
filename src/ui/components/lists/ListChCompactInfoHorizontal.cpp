#include "ListChCompactInfoHorizontal.h"
#include "../ChCompactInfo.h"
#include "ui/elements/HorizontalList.h"

namespace ui {

ListChCompactInfoHorizontal::ListChCompactInfoHorizontal(sdl2w::Window* _window,
                                                         UiElement* _parent)
    : UiElement(_window, _parent) {
  style.fontSize = sdl2w::TEXT_SIZE_18;
}

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
  defaultChCompactInfo.getStyle().scale = style.scale;
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

  for (size_t i = 0; i < props.entries.size(); i++) {
    auto chCompactInfo = new ChCompactInfo(window, this);
    auto& s = chCompactInfo->getStyle();
    s.width = style.width;
    s.x = style.x;
    s.y = style.y;
    s.scale = style.scale;
    s.fontSize = style.fontSize;
    auto chCompactInfoProps = props.entries[i];
    chCompactInfoProps.numStatusColumns = numStatusColumns;
    chCompactInfoProps.isSelected = static_cast<int>(i) == props.selectedIndex;
    chCompactInfo->setProps(chCompactInfoProps);
    list->addChild(chCompactInfo);
  }

  auto& listStyle = list->getStyle();
  listStyle.x = style.x;
  listStyle.y = style.y;
  listStyle.height = chCompactInfoScaledHeight;
  listStyle.scale = 1.;

  HorizontalListProps listProps;
  listProps.lineWidth = chCompactInfoScaledWidth;
  listProps.lineGap = static_cast<int>(props.lineGap * style.scale);
  list->setProps(listProps);

  addChild(list);
}

void ListChCompactInfoHorizontal::render(int dt) { UiElement::render(dt); }

} // namespace ui
