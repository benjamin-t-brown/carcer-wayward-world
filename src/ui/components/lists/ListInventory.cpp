#include "ListInventory.h"
#include "ui/elements/VerticalList.h"
#include "ui/colors.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonList.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/observers/ObserverInventorySelectItem.hpp"
#include "ui/observers/ObserverReorderInventoryItem.hpp"
#include "ui/observers/ObserverShowLayerInventoryContext.hpp"

namespace ui {

// ListInventory Implementation
ListInventory::ListInventory(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListInventory::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListInventory::setProps(const ListInventoryProps& _props) {
  props = _props;
  build();
}

const ListInventoryProps& ListInventory::getProps() const { return props; }

UiElement* ListInventory::createItemElement(const ListInventoryPropsItem& item,
                                            int index) {
  auto container = new Quad(window, this);
  container->setId(item.itemName);
  auto& s = container->getStyle();
  s.width = style.width * style.scale;
  s.height = props.lineHeight * style.scale;
  s.scale = 1.0f;
  container->setProps({
      .bgColor = Colors::Transparent,
  });

  const int scaledIndexColumnWidth = static_cast<int>(indexColumnWidth * style.scale);
  const int scaledIndexPaddingLeft = static_cast<int>(indexPaddingLeft * style.scale);
  const int scaledReorderBtnWidth = static_cast<int>(reorderBtnWidth * style.scale);
  // const int scaledReorderBtnHeight = static_cast<int>(reorderBtnHeight * style.scale);
  const int scaledReorderBtnGap = static_cast<int>(reorderBtnGap * style.scale);
  const int scaledReorderColumnGap = static_cast<int>(reorderColumnGap * style.scale);
  const int scaledReorderColumnWidth =
      scaledReorderBtnWidth + scaledReorderBtnGap + scaledReorderBtnWidth;
  const int indexColumnStartX =
      scaledReorderColumnGap + scaledReorderColumnWidth + scaledReorderColumnGap;
  const int numberGap = static_cast<int>(4 * style.scale);
  const int labelGapAfterIcon = static_cast<int>(12 * style.scale);
  const float iconScale = 2.f;
  const int scaledIconWidth = static_cast<int>(iconSpriteSize * iconScale * style.scale);

  const int reorderBtnY =
      ButtonList::yForListRow(s.height, reorderBtnHeight, style.scale);
  const int reorderButtonsX = scaledReorderColumnGap;

  auto upBtn = new ButtonList(window, this);
  upBtn->setId("reorderUp");
  auto& upBtnStyle = upBtn->getStyle();
  upBtnStyle.x = reorderButtonsX;
  upBtnStyle.y = reorderBtnY;
  upBtnStyle.width = reorderBtnWidth;
  upBtnStyle.height = reorderBtnHeight;
  upBtnStyle.scale = style.scale;
  upBtn->setProps({.arrow = ScrollDirection::UP,
                   .bgColor = Colors::Transparent,
                   .bgColorTopRight = Colors::Transparent,
                   .bgColorBottomLeft = Colors::Transparent,
                   .arrowColor = Colors::Black});
  if (!props.characterPlayerId.empty() && index > 0) {
    upBtn->addEventObserver(
        new ObserverReorderInventoryItem(props.characterPlayerId, index, -1));
  }
  container->addChild(upBtn);

  auto downBtn = new ButtonList(window, this);
  downBtn->setId("reorderDown");
  auto& downBtnStyle = downBtn->getStyle();
  downBtnStyle.x = reorderButtonsX + scaledReorderBtnWidth + scaledReorderBtnGap;
  downBtnStyle.y = reorderBtnY;
  downBtnStyle.width = reorderBtnWidth;
  downBtnStyle.height = reorderBtnHeight;
  downBtnStyle.scale = style.scale;
  downBtn->setProps({.arrow = ScrollDirection::DOWN,
                     .bgColor = Colors::Transparent,
                     .bgColorTopRight = Colors::Transparent,
                     .bgColorBottomLeft = Colors::Transparent,
                     .arrowColor = Colors::Black});
  if (!props.characterPlayerId.empty() &&
      index + 1 < static_cast<int>(props.items.size())) {
    downBtn->addEventObserver(
        new ObserverReorderInventoryItem(props.characterPlayerId, index, 1));
  }
  container->addChild(downBtn);

  auto indexLine = new TextLine(window, this);
  indexLine->setId("index");
  auto& indexStyle = indexLine->getStyle();
  setBaseFontConfig(indexStyle, BaseFontConfig::MODAL_TEXT);
  indexStyle.fontSize = sdl2w::TEXT_SIZE_18;
  indexStyle.fontColor = Colors::Black;
  indexStyle.textAlign = TextAlign::LEFT_CENTER;
  indexStyle.scale = 1.0f;
  indexStyle.y = s.height / 2;
  indexLine->setProps({
      .textBlocks =
          {
              {
                  .text = bmin::toString(index + 1) + ".",
              },
          },
  });
  const int indexTextWidth = indexLine->getDims().first;
  indexStyle.x = indexColumnStartX + scaledIndexPaddingLeft +
                 (scaledIndexColumnWidth - scaledIndexPaddingLeft - indexTextWidth);
  indexLine->build();
  container->addChild(indexLine);

  auto icon = new Quad(window, this);
  icon->setId("icon");
  auto& iconStyle = icon->getStyle();
  iconStyle.width = iconSpriteSize;
  iconStyle.height = iconSpriteSize;
  iconStyle.x = indexColumnStartX + scaledIndexColumnWidth + numberGap;
  iconStyle.y = (s.height - iconStyle.height * style.scale * iconScale) / 2;
  iconStyle.scale = iconScale * style.scale;
  icon->setProps({
      .bgColor = {66, 202, 253, 50},
      .bgSprite = item.itemSprite,
  });
  container->addChild(icon);

  const int scaledContextBtnSize = contextBtnSize * style.scale;
  const int labelX = iconStyle.x + scaledIconWidth + labelGapAfterIcon;
  const int labelContextGap = static_cast<int>(4 * style.scale);
  const SDL_Color labelColor = item.isEquipped ? Colors::Blue : Colors::Black;
  auto fontConfig =
      item.isEquipped ? BaseFontConfig::MODAL_TEXT_BOLD : BaseFontConfig::MODAL_TEXT;
  String itemDisplayLabel = item.itemLabel;
  if (item.isStackable) {
    itemDisplayLabel += " (" + bmin::toString(item.quantity) + ")";
  }

  if (item.isEquippable && !props.characterPlayerId.empty()) {
    auto label = new ButtonTextWrap(window, this);
    label->setId("label");
    auto& labelStyle = label->getStyle();
    setBaseFontConfig(labelStyle, fontConfig);
    labelStyle.fontSize = sdl2w::TEXT_SIZE_18;
    labelStyle.fontColor = labelColor;
    labelStyle.scale = 1.0f;
    labelStyle.x = labelX;
    labelStyle.y = 0;
    labelStyle.width = static_cast<int>(
        (s.width - scaledContextBtnSize - labelX - labelContextGap) / labelStyle.scale);
    label->setStyle(labelStyle);
    label->setProps({
        .text = item.itemName,
    });
    const int textOnlyHeight = label->getDims().second;
    const int verticalPadding = std::max(
        0, static_cast<int>((s.height - textOnlyHeight) / (2 * labelStyle.scale)));
    label->setProps({
        .text = itemDisplayLabel,
        .verticalPadding = verticalPadding,
    });
    label->addEventObserver(
        new ui::ObserverInventorySelectItem(props.characterPlayerId, item.itemId));
    label->build();
    container->addChild(label);
  } else {
    auto label = new TextLine(window, this);
    label->setId("label");
    auto& labelStyle = label->getStyle();
    setBaseFontConfig(labelStyle, BaseFontConfig::MODAL_TEXT);
    labelStyle.fontSize = sdl2w::TEXT_SIZE_18;
    labelStyle.fontColor = labelColor;
    labelStyle.textAlign = TextAlign::LEFT_CENTER;
    labelStyle.scale = 1.0f;
    labelStyle.x = labelX;
    labelStyle.y = s.height / 2;
    label->setProps({
        .textBlocks =
            {
                {
                    .text = itemDisplayLabel,
                    .fontColor = labelColor,
                },
            },
    });
    container->addChild(label);
  }

  // Create context button
  auto contextBtn = new ButtonModal(window, this);
  contextBtn->setId("contextBtn");
  auto& contextBtnStyle = contextBtn->getStyle();
  contextBtnStyle.x = s.width - scaledContextBtnSize;
  contextBtnStyle.y = (s.height - contextBtnSize) / 2;
  contextBtnStyle.width = scaledContextBtnSize;
  contextBtnStyle.height = scaledContextBtnSize;
  contextBtnStyle.scale = 1.0f;
  contextBtn->setProps({
      .text = "*",
  });
  contextBtn->addEventObserver(
      new ui::ObserverShowLayerInventoryContext(window, item.itemName, item.itemId));
  container->addChild(contextBtn);

  return container;
}

void ListInventory::build() {
  children.clear();

  // Create VerticalList component
  auto list = new VerticalList(window, this);
  list->setId("list");
  auto& s = list->getStyle();
  s.x = style.x;
  s.y = style.y + static_cast<int>(props.paddingTop * style.scale);
  s.width = style.width * style.scale;
  s.scale = 1.0f;

  for (size_t i = 0; i < props.items.size(); ++i) {
    list->addChild(createItemElement(props.items[i], static_cast<int>(i)));
  }

  list->setProps({
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = props.lineGap,
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListInventory::render(int dt) {
  //
  UiElement::render(dt);
}

} // namespace ui
