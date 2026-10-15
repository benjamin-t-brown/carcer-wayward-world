#include "ListInventory.h"
#include "ui/colors.hpp"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/VerticalList.h"
#include "ui/elements/buttons/ButtonList.h"
#include "ui/elements/buttons/ButtonSprite.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/helpers/uiSounds.h"
#include "ui/observers/ActionObserver.hpp"
#include "actions/navigation/UiToggleEquipInventoryItem.hpp"
#include "actions/navigation/UiReorderInventoryItem.hpp"
#include "actions/navigation/UiShowLayerInventoryContext.hpp"
#include "actions/navigation/UiUseInventoryItem.hpp"

namespace ui {

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
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListInventoryProps& ListInventory::getProps() const { return props; }

UiElement* ListInventory::createItemElement(const ListInventoryPropsItem& item,
                                            int index) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(item.itemName);
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
  const float iconScale = 2.f;
  const int scaledIconWidth = static_cast<int>(iconSpriteSize * iconScale * style.scale);

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
  if (!props.characterPlayerId.empty() && index > 0) {
    upBtn->addEventObserver(
        ui::makeActionObserver<state::actions::UiReorderInventoryItem>(
            props.characterPlayerId, index, -1));
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
  if (!props.characterPlayerId.empty() &&
      index + 1 < static_cast<int>(props.items.size())) {
    downBtn->addEventObserver(
        ui::makeActionObserver<state::actions::UiReorderInventoryItem>(
            props.characterPlayerId, index, 1));
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
      .bgSprite = item.itemSprite,
  });
  container->addChild(bmin::UniquePtr<ui::UiElement>(icon));

  // icons_for_buttons sprites are 16x16; padding 8 → 32px button (matches row).
  constexpr int kActionIconSize = 16;
  constexpr int kActionBtnPadding = 8;
  const int actionBtnLogicalSize = kActionIconSize + kActionBtnPadding * 2;
  const int scaledContextBtnSize =
      static_cast<int>(actionBtnLogicalSize * style.scale);
  const int labelX = iconX + scaledIconWidth + labelGapAfterIcon;
  const int labelContextGap = static_cast<int>(4 * style.scale);
  const int useBtnGap = static_cast<int>(4 * style.scale);
  const auto showUseBtn =
      item.isUsableNow && !props.characterPlayerId.empty();
  const int trailingButtonsWidth =
      scaledContextBtnSize +
      (showUseBtn ? scaledContextBtnSize + useBtnGap : 0);
  const SDL_Color labelColor = item.isEquipped ? Colors::Blue : Colors::Black;
  auto fontConfig =
      item.isEquipped ? BaseFontConfig::MODAL_TEXT_BOLD : BaseFontConfig::MODAL_TEXT;
  bmin::String itemDisplayLabel = item.itemLabel;
  if (item.isStackable) {
    itemDisplayLabel += " (" + bmin::toString(item.quantity) + ")";
  }
  if (!item.equippedSlotAbbrev.empty()) {
    itemDisplayLabel += " [" + item.equippedSlotAbbrev + "]";
  }

  TextFontProps labelFont;
  setBaseFontConfig(labelFont, fontConfig);

  if (item.isEquippable && !props.characterPlayerId.empty()) {
    const int labelWidth = static_cast<int>(
        (rowWidth - trailingButtonsWidth - labelX - labelContextGap));
    auto label = new ButtonTextWrap(window, this);
    label->setId("label");
    label->setPos(labelX, 0);
    label->setScale(1.0f);
    label->setProps(ButtonTextWrapProps{
        .textParagraph =
            {
                .textBlocks = {{.text = item.itemName, .fontColor = labelColor}},
                .width = labelWidth,
                .fontFamily = labelFont.fontFamily,
                .fontSize = sdl2w::TEXT_SIZE_18,
                .fontColor = labelColor,
            },
    });
    const int textOnlyHeight = label->getDims().second;
    const int verticalPadding =
        std::max(0, static_cast<int>((rowHeight - textOnlyHeight) / 2));
    label->setProps(ButtonTextWrapProps{
        .verticalPadding = verticalPadding,
        .textParagraph =
            {
                .textBlocks = {{.text = itemDisplayLabel, .fontColor = labelColor}},
                .width = labelWidth,
                .fontFamily = labelFont.fontFamily,
                .fontSize = sdl2w::TEXT_SIZE_18,
                .fontColor = labelColor,
            },
    });
    label->clickSoundName = kButton2SoundName;
    label->addEventObserver(
        ui::makeActionObserver<state::actions::UiToggleEquipInventoryItem>(
            props.characterPlayerId, item.itemId));
    container->addChild(bmin::UniquePtr<ui::UiElement>(label));
  } else {
    auto label = new TextLine(window, this);
    label->setId("label");
    label->setPos(labelX, rowHeight / 2);
    label->setScale(1.0f);
    TextLineProps labelProps;
    labelProps.fontFamily = labelFont.fontFamily;
    labelProps.fontSize = sdl2w::TEXT_SIZE_18;
    labelProps.fontColor = labelColor;
    labelProps.textAlign = TextAlign::LEFT_CENTER;
    labelProps.textBlocks.pushBack({
        .text = itemDisplayLabel,
        .fontColor = labelColor,
    });
    label->setProps(labelProps);
    container->addChild(bmin::UniquePtr<ui::UiElement>(label));
  }

  const int contextBtnX = rowWidth - scaledContextBtnSize;
  const int rowBtnY = (rowHeight - scaledContextBtnSize) / 2;

  if (showUseBtn) {
    auto useBtn = new ButtonSprite(window, this);
    useBtn->setId("useBtn");
    useBtn->setPos(contextBtnX - useBtnGap - scaledContextBtnSize, rowBtnY);
    useBtn->setScale(style.scale);
    useBtn->setProps(ButtonSpriteProps{
        .spriteName = "icons_for_buttons_1",
        .spriteWidth = kActionIconSize,
        .spriteHeight = kActionIconSize,
        .padding = kActionBtnPadding,
    });
    useBtn->addEventObserver(
        ui::makeParallelActionObserver<state::actions::UiUseInventoryItem>(
            props.characterPlayerId, item.itemId));
    container->addChild(bmin::UniquePtr<ui::UiElement>(useBtn));
  }

  auto contextBtn = new ButtonSprite(window, this);
  contextBtn->setId("contextBtn");
  contextBtn->setPos(contextBtnX, rowBtnY);
  contextBtn->setScale(style.scale);
  contextBtn->setProps(ButtonSpriteProps{
      .spriteName = "icons_for_buttons_0",
      .spriteWidth = kActionIconSize,
      .spriteHeight = kActionIconSize,
      .padding = kActionBtnPadding,
  });
  contextBtn->addEventObserver(
      ui::makeActionObserver<state::actions::UiShowLayerInventoryContext>(
          window, item.itemName, item.itemId));
  container->addChild(bmin::UniquePtr<ui::UiElement>(contextBtn));

  return container;
}

void ListInventory::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  list->setPos(style.x, style.y + static_cast<int>(props.paddingTop * style.scale));
  list->setScale(1.0f);

  for (size_t i = 0; i < props.items.size(); ++i) {
    list->addChild(bmin::UniquePtr<ui::UiElement>(createItemElement(props.items[i], static_cast<int>(i))));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = props.lineGap,
      .bgColor = Colors::Transparent,
  });

  addChild(bmin::UniquePtr<ui::UiElement>(list));
}

void ListInventory::render(int dt) { UiElement::render(dt); }

} // namespace ui
