#include "ListSpecialItems.h"
#include "actions/navigation/UiReorderSpecialItem.hpp"
#include "actions/navigation/UiShowLayerSpecialItemContext.hpp"
#include "ui/colors.hpp"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/VerticalList.h"
#include "ui/elements/buttons/ButtonList.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ActionObserver.hpp"

namespace ui {

ListSpecialItems::ListSpecialItems(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListSpecialItems::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListSpecialItems::setProps(const ListSpecialItemsProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListSpecialItemsProps& ListSpecialItems::getProps() const { return props; }

UiElement* ListSpecialItems::createItemElement(const ListSpecialItemsPropsItem& listItem,
                                               int index) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(listItem.itemName);
  container->setScale(1.0f);
  container->setProps(QuadProps{
      .width = rowWidth,
      .height = rowHeight,
      .bgColor = Colors::Transparent,
  });

  const int scaledIndexColumnWidth = static_cast<int>(indexColumnWidth * style.scale);
  const int scaledIndexPaddingLeft = static_cast<int>(indexPaddingLeft * style.scale);
  const int scaledReorderBtnWidth = static_cast<int>(reorderBtnWidth * style.scale);
  const int scaledReorderBtnGap = static_cast<int>(reorderBtnGap * style.scale);
  const int scaledReorderColumnGap = static_cast<int>(reorderColumnGap * style.scale);
  const int scaledReorderColumnWidth =
      scaledReorderBtnWidth + scaledReorderBtnGap + scaledReorderBtnWidth;
  const int indexColumnStartX =
      scaledReorderColumnGap + scaledReorderColumnWidth + scaledReorderColumnGap;
  const int numberGap = static_cast<int>(4 * style.scale);
  const int labelGapAfterIcon = static_cast<int>(12 * style.scale);

  const int reorderBtnY =
      ButtonList::yForListRow(rowHeight, reorderBtnHeight, style.scale);
  const int reorderButtonsX = scaledReorderColumnGap;

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  auto upBtn = new ButtonList(window, this);
  upBtn->setId("reorderUp");
  upBtn->setPos(reorderButtonsX, reorderBtnY);
  upBtn->setScale(style.scale);
  upBtn->setProps(ButtonListProps{
      .arrow = ScrollDirection::UP,
      .width = reorderBtnWidth,
      .height = reorderBtnHeight,
      .bgColor = Colors::Transparent,
      .bgColorTopRight = Colors::Transparent,
      .bgColorBottomLeft = Colors::Transparent,
      .arrowColor = Colors::Black,
  });
  if (index > 0) {
    upBtn->addEventObserver(
        ui::makeActionObserver<state::actions::UiReorderSpecialItem>(index, -1));
  }
  container->addChild(bmin::UniquePtr<ui::UiElement>(upBtn));

  auto downBtn = new ButtonList(window, this);
  downBtn->setId("reorderDown");
  downBtn->setPos(reorderButtonsX + scaledReorderBtnWidth + scaledReorderBtnGap,
                  reorderBtnY);
  downBtn->setScale(style.scale);
  downBtn->setProps(ButtonListProps{
      .arrow = ScrollDirection::DOWN,
      .width = reorderBtnWidth,
      .height = reorderBtnHeight,
      .bgColor = Colors::Transparent,
      .bgColorTopRight = Colors::Transparent,
      .bgColorBottomLeft = Colors::Transparent,
      .arrowColor = Colors::Black,
  });
  if (index + 1 < static_cast<int>(props.items.size())) {
    downBtn->addEventObserver(
        ui::makeActionObserver<state::actions::UiReorderSpecialItem>(index, 1));
  }
  container->addChild(bmin::UniquePtr<ui::UiElement>(downBtn));

  auto indexLine = new TextLine(window, this);
  indexLine->setId("index");
  indexLine->setPos(0, rowHeight / 2);
  indexLine->setScale(1.0f);
  TextLineProps indexProps;
  indexProps.fontFamily = font.fontFamily;
  indexProps.fontSize = sdl2w::TEXT_SIZE_18;
  indexProps.fontColor = Colors::Black;
  indexProps.textAlign = TextAlign::LEFT_CENTER;
  indexProps.textBlocks.pushBack({.text = bmin::toString(index + 1) + "."});
  indexLine->setProps(indexProps);
  const int indexTextWidth = indexLine->getDims().first;
  indexLine->setPos(
      indexColumnStartX + scaledIndexPaddingLeft +
          (scaledIndexColumnWidth - scaledIndexPaddingLeft - indexTextWidth),
      rowHeight / 2);
  container->addChild(bmin::UniquePtr<ui::UiElement>(indexLine));

  const int iconX = indexColumnStartX + scaledIndexColumnWidth + numberGap;
  const int iconY =
      (rowHeight - static_cast<int>(iconSpriteSize * style.scale * iconScale)) / 2;

  auto icon = new Quad(window, this);
  icon->setId("icon");
  icon->setPos(iconX, iconY);
  icon->setScale(iconScale * style.scale);
  icon->setProps(QuadProps{
      .width = iconSpriteSize,
      .height = iconSpriteSize,
      .bgColor = {66, 202, 253, 50},
      .bgSprite = listItem.itemSprite,
  });
  container->addChild(bmin::UniquePtr<ui::UiElement>(icon));

  const int scaledContextBtnSize = contextBtnSize * style.scale;
  const int scaledIconWidth = static_cast<int>(iconSpriteSize * iconScale * style.scale);
  const int labelX = iconX + scaledIconWidth + labelGapAfterIcon;

  auto contextBtn = new ButtonModal(window, this);
  contextBtn->setId("contextBtn");
  contextBtn->setPos(rowWidth - scaledContextBtnSize,
                     (rowHeight - scaledContextBtnSize) / 2);
  contextBtn->setScale(1.0f);
  contextBtn->setProps(ButtonModalProps{
      .text = "*",
      .width = scaledContextBtnSize,
      .height = scaledContextBtnSize,
  });
  contextBtn->addEventObserver(
      ui::makeActionObserver<state::actions::UiShowLayerSpecialItemContext>(
          window, listItem.itemName, listItem.quantity));
  container->addChild(bmin::UniquePtr<ui::UiElement>(contextBtn));

  auto label = new TextLine(window, this);
  label->setId("label");
  label->setPos(labelX, rowHeight / 2);
  label->setScale(1.0f);
  label->setProps(TextLineProps{
      .textBlocks = {{.text = listItem.itemLabel}},
      .fontFamily = font.fontFamily,
      .fontSize = sdl2w::TEXT_SIZE_18,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  container->addChild(bmin::UniquePtr<ui::UiElement>(label));

  return container;
}

void ListSpecialItems::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  if (props.items.empty()) {
    return;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  list->setPos(style.x, style.y + static_cast<int>(props.paddingTop * style.scale));
  list->setScale(1.0f);

  for (size_t i = 0; i < props.items.size(); ++i) {
    list->addChild(bmin::UniquePtr<ui::UiElement>(
        createItemElement(props.items[i], static_cast<int>(i))));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(bmin::UniquePtr<ui::UiElement>(list));
}

void ListSpecialItems::render(int dt) { UiElement::render(dt); }

} // namespace ui
