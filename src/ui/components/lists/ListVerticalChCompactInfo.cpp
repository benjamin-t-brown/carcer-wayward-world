#include "ListVerticalChCompactInfo.h"
#include "../ChCompactInfo.h"
#include "../VerticalList.h"

namespace ui {

ListVerticalChCompactInfo::ListVerticalChCompactInfo(sdl2w::Window* _window,
                                                     UiElement* _parent)
    : UiElement(_window, _parent) {
  style.fontSize = sdl2w::TEXT_SIZE_18;
}

const std::pair<int, int> ListVerticalChCompactInfo::getDims() const {
  if (children.empty()) {
    return {style.width, 0};
  }

  return children[0]->getDims();
}

void ListVerticalChCompactInfo::setProps(const ListVerticalChCompactInfoProps& _props) {
  props = _props;
  build();
}

const ListVerticalChCompactInfoProps& ListVerticalChCompactInfo::getProps() const {
  return props;
}

void ListVerticalChCompactInfo::build() {
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

void ListVerticalChCompactInfo::render(int dt) { UiElement::render(dt); }

} // namespace ui
