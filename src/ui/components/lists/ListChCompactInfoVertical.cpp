#include "ListChCompactInfoVertical.h"
#include "../ChCompactInfo.h"
#include "../VerticalList.h"

namespace ui {

ListChCompactInfoVertical::ListChCompactInfoVertical(sdl2w::Window* _window,
                                                     UiElement* _parent)
    : UiElement(_window, _parent) {
  style.fontSize = sdl2w::TEXT_SIZE_18;
}

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
  defaultChCompactInfo.getStyle().scale = style.scale;
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
  listStyle.width = chCompactInfoScaledWidth;
  listStyle.scale = 1.;

  VerticalListProps listProps;
  listProps.lineHeight = chCompactInfoScaledHeight;
  listProps.lineGap = static_cast<int>(props.lineGap * style.scale);
  list->setProps(listProps);

  addChild(list);
}

void ListChCompactInfoVertical::render(int dt) { UiElement::render(dt); }

} // namespace ui
