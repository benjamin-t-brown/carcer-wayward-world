module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <limits>

export module carcer.ui.widgets.composites;
export import bmin.containers;
export import carcer.ui.core;
export import carcer.ui.widgets.primitives;
export import carcer.ui.widgets.controls;
export import carcer.model;
export import carcer.ui.widgets.views;
export import carcer.state;
import carcer.actions;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

// --- from ui/components/lists/ListInventory.h ---
namespace ui {

struct ListInventoryPropsItem {
  bmin::String itemId;
  bmin::String itemName;
  bmin::String itemLabel;
  bmin::String itemSprite;
  bool isEquippable = false;
  bool isEquipped = false;
  bmin::String equippedSlotAbbrev;
  bool isStackable = false;
  int quantity = 1;
};
struct ListInventoryProps {
  bmin::String characterPlayerId;
  bmin::DynArray<ListInventoryPropsItem> items;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

// ListInventory - renders a vertical list of inventory items
class ListInventory : public UiElement {
private:
  ListInventoryProps props;

  const int contextBtnSize = 32;
  const int iconSpriteSize = 16;
  const int indexColumnWidth = 28;
  const int indexPaddingLeft = 4;
  const int reorderBtnHeight = 28;
  const int reorderBtnWidth = 14;
  const int reorderBtnGap = 2;
  const int reorderColumnGap = 2;

  UiElement* createItemElement(const ListInventoryPropsItem& item, int index);

public:
  ListInventory(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListInventory() override = default;

  void setProps(const ListInventoryProps& props);
  const ListInventoryProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverInventorySelectItem.hpp ---
namespace ui {
class ObserverInventorySelectItem : public ui::UiEventObserver,
                                    public state::StateManagerInterface {

  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverInventorySelectItem(const bmin::String& _characterPlayerId,
                              const bmin::String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};
} // namespace ui

// --- from ui/observers/ObserverReorderInventoryItem.hpp ---
namespace ui {

class ObserverReorderInventoryItem : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  bmin::String characterPlayerId;
  int inventoryIndex;
  int direction;

public:
  ObserverReorderInventoryItem(const bmin::String& _characterPlayerId,
                               int _inventoryIndex,
                               int _direction)
      : characterPlayerId(_characterPlayerId),
        inventoryIndex(_inventoryIndex),
        direction(_direction) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerInventoryContext.hpp ---
namespace ui {
class ObserverShowLayerInventoryContext : public ui::UiEventObserver,
                                          public state::StateManagerInterface {

  sdl2w::Window* window;
  bmin::String itemName;
  bmin::String itemId;

public:
  ObserverShowLayerInventoryContext(sdl2w::Window* _window,
                                    const bmin::String& itemName,
                                    const bmin::String& itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};
} // namespace ui

} // export

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
        new ObserverReorderInventoryItem(props.characterPlayerId, index, -1));
  }
  container->addChild(upBtn);

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
        new ObserverReorderInventoryItem(props.characterPlayerId, index, 1));
  }
  container->addChild(downBtn);

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
  container->addChild(indexLine);

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
  container->addChild(icon);

  const int scaledContextBtnSize = contextBtnSize * style.scale;
  const int labelX = iconX + scaledIconWidth + labelGapAfterIcon;
  const int labelContextGap = static_cast<int>(4 * style.scale);
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
    const int labelWidth =
        static_cast<int>((rowWidth - scaledContextBtnSize - labelX - labelContextGap));
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
    label->addEventObserver(
        new ui::ObserverInventorySelectItem(props.characterPlayerId, item.itemId));
    container->addChild(label);
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
    container->addChild(label);
  }

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
      new ui::ObserverShowLayerInventoryContext(window, item.itemName, item.itemId));
  container->addChild(contextBtn);

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
    list->addChild(createItemElement(props.items[i], static_cast<int>(i)));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = props.lineGap,
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListInventory::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverInventorySelectItem::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverInventorySelectItem::onClick " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::toggleEquipInventoryItem(characterPlayerId, itemId),
        0);
  }

void ObserverReorderInventoryItem::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverReorderInventoryItem::onClick index=" << inventoryIndex
              << " direction=" << direction << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::reorderInventoryItem(
            characterPlayerId, inventoryIndex, direction),
        0);
  }

void ObserverShowLayerInventoryContext::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverShowLayerInventoryContext::onClick " << itemName << " " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::showLayerInventoryContext(window, itemName, itemId),
        0);
  }

} // namespace ui

export {

namespace ui {

struct ListMagicSpellsPropsSpell {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  // GCC BMI: DynArray nested in DynArray under UiElement corrupts GCM.
  std::vector<bmin::String> requiredRuneSprites;
};

struct ListMagicSpellsProps {
  bmin::DynArray<ListMagicSpellsPropsSpell> spells;
  bmin::String casterId;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
  bool enableSpellInfoOnClick = true;
  bool enableSpellCastOnClick = false;
};

class ListMagicSpells : public UiElement {
private:
  ListMagicSpellsProps props;
  static constexpr int iconSpriteSize = 24;
  static constexpr float iconScale = 1.f;
  static constexpr int labelGapAfterIcon = 12;
  static constexpr int requiredRuneIconSize = 24;
  static constexpr float requiredRuneIconScale = 1.f;
  static constexpr int requiredRuneGap = 2;
  static constexpr int requiredRunesRightPadding = 4;
  UiElement* createSpellElement(const ListMagicSpellsPropsSpell& spell);

public:
  ListMagicSpells(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListMagicSpells() override = default;
  void setProps(const ListMagicSpellsProps& props);
  const ListMagicSpellsProps& getProps() const;
  const std::pair<int, int> getDims() const override;
  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverSelectSpellCast.hpp ---
namespace ui {

class ObserverSelectSpellCast : public ui::UiEventObserver,
                                public state::StateManagerInterface {
  bmin::String spellId;
  bmin::String chId;

public:
  explicit ObserverSelectSpellCast(const bmin::String& _spellId,
                                   const bmin::String& _chId)
      : spellId(_spellId), chId(_chId) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerSpellInfo.hpp ---
namespace ui {

class ObserverShowLayerSpellInfo : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String spellName;

public:
  ObserverShowLayerSpellInfo(sdl2w::Window* _window, const bmin::String& _spellName)
      : window(_window), spellName(_spellName) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

namespace ui {

ListMagicSpells::ListMagicSpells(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListMagicSpells::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListMagicSpells::setProps(const ListMagicSpellsProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListMagicSpellsProps& ListMagicSpells::getProps() const { return props; }

UiElement* ListMagicSpells::createSpellElement(const ListMagicSpellsPropsSpell& spell) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(spell.id);
  container->setScale(1.0f);
  container->setProps(QuadProps{
      .width = rowWidth,
      .height = rowHeight,
      .bgColor = Colors::Transparent,
  });
  if (props.enableSpellCastOnClick && !spell.id.empty()) {
    container->addEventObserver(new ObserverSelectSpellCast(spell.id, props.casterId));
  } else if (props.enableSpellInfoOnClick && !spell.id.empty()) {
    container->addEventObserver(new ObserverShowLayerSpellInfo(window, spell.id));
  }

  // drawSprite uses native sprite w/h (props width/height are ignored), so center
  // and label offset must use the sprite's drawn size, not iconSpriteSize.
  int iconDrawW = 0;
  int iconDrawH = 0;
  if (!spell.iconSprite.empty()) {
    const auto& sprite =
        window->getStore().getSprite(bmin::toStringView(spell.iconSprite));
    iconDrawW = static_cast<int>(sprite.w * iconScale * style.scale);
    iconDrawH = static_cast<int>(sprite.h * iconScale * style.scale);

    auto icon = new SpriteElement(window, this);
    icon->setId("icon");
    icon->setPos(0, (rowHeight - iconDrawH) / 2);
    icon->setScale(iconScale * style.scale);
    icon->setProps(SpriteElementProps{
        .width = sprite.w,
        .height = sprite.h,
        .spriteName = spell.iconSprite,
    });
    container->addChild(icon);
  }

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  const int labelX = iconDrawW + static_cast<int>(labelGapAfterIcon * style.scale);

  auto label = new TextLine(window, this);
  label->setId("label");
  label->setPos(labelX, rowHeight / 2);
  label->setScale(1.0f);
  TextLineProps labelProps;
  labelProps.fontFamily = font.fontFamily;
  labelProps.fontSize = sdl2w::TEXT_SIZE_18;
  labelProps.fontColor = Colors::Black;
  labelProps.textAlign = TextAlign::LEFT_CENTER;
  labelProps.textBlocks.pushBack({.text = spell.label});
  label->setProps(labelProps);
  container->addChild(label);

  const int scaledRequiredRuneSize =
      static_cast<int>(requiredRuneIconSize * requiredRuneIconScale * style.scale);
  const int scaledRequiredRuneGap = static_cast<int>(requiredRuneGap * style.scale);
  const int scaledRightPadding =
      static_cast<int>(requiredRunesRightPadding * style.scale);
  const int requiredCount = static_cast<int>(spell.requiredRuneSprites.size());
  if (requiredCount > 0) {
    const int totalRequiredWidth = requiredCount * scaledRequiredRuneSize +
                                   (requiredCount - 1) * scaledRequiredRuneGap;
    int runeX = rowWidth - scaledRightPadding - totalRequiredWidth;
    const int runeY = (rowHeight - scaledRequiredRuneSize) / 2;
    for (int i = 0; i < requiredCount; ++i) {
      const auto& runeSprite = spell.requiredRuneSprites[static_cast<size_t>(i)];
      if (runeSprite.empty()) {
        runeX += scaledRequiredRuneSize + scaledRequiredRuneGap;
        continue;
      }
      auto runeIcon = new SpriteElement(window, this);
      runeIcon->setId("requiredRune_" + bmin::toString(i));
      runeIcon->setPos(runeX, runeY);
      runeIcon->setScale(requiredRuneIconScale * style.scale);
      runeIcon->setProps(SpriteElementProps{
          .width = requiredRuneIconSize,
          .height = requiredRuneIconSize,
          .spriteName = runeSprite,
      });
      container->addChild(runeIcon);
      runeX += scaledRequiredRuneSize + scaledRequiredRuneGap;
    }
  }

  return container;
}

void ListMagicSpells::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  if (props.spells.empty()) {
    return;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  list->setPos(style.x, style.y + static_cast<int>(props.paddingTop * style.scale));
  list->setScale(1.0f);

  for (size_t i = 0; i < props.spells.size(); i++) {
    list->addChild(createSpellElement(props.spells[i]));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListMagicSpells::render(int dt) { UiElement::render(dt); }

void ObserverSelectSpellCast::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
  LOG(INFO) << "ObserverSelectSpellCast::onClick " << spellId << LOG_ENDL;
  auto stateManager = getStateManager();
  if (!stateManager || spellId.empty()) {
    return;
  }
  stateManager->enqueueAction(stateManager->getActionData(),
                              state::actions::selectSpellCast(spellId, chId),
                              0);
}

void ObserverShowLayerSpellInfo::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
  LOG(INFO) << "ObserverShowLayerSpellInfo::onClick " << spellName << LOG_ENDL;
  auto stateManager = getStateManager();
  if (!stateManager || spellName.empty()) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      state::actions::showLayerSpellInfo(window, spellName),
      0);
}

} // namespace ui

export {

// --- from ui/components/lists/ListPickUp.h ---
namespace ui {

struct ListPickUpPropsItem {
  model::ItemInstance item;
  bmin::String itemLabel;
  int weight = 1;
  bmin::String itemSprite;
};

struct ListPickUpProps {
  bmin::DynArray<ListPickUpPropsItem> items;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

// ListPickUp - renders a vertical list of items that can be picked up
class ListPickUp : public UiElement {
private:
  ListPickUpProps props;

  static constexpr int contextBtnSize = 32;
  static constexpr int iconSpriteSize = 16;
  static constexpr float iconScale = 2.f;
  static constexpr int maxShortcutItems = 26;
  static constexpr int shortcutGapAfterIcon = 4;
  static constexpr int labelGapAfterShortcut = 8;
  static constexpr int labelGapAfterIconNoShortcut = 24;

  UiElement* createItemElement(const ListPickUpPropsItem& item, int index);
  static bmin::String shortcutLetterForIndex(int index);

public:
  ListPickUp(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListPickUp() override = default;

  void setProps(const ListPickUpProps& props);
  const ListPickUpProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverPickUpItem.hpp ---
namespace ui {

class ObserverPickUpItem : public ui::UiEventObserver,
                           public state::StateManagerInterface {
  bmin::String itemId;

public:
  explicit ObserverPickUpItem(const model::ItemInstance& item) : itemId(item.id) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerPickUpContext.hpp ---
namespace ui {
class ObserverShowLayerPickUpContext : public ui::UiEventObserver,
                                       public state::StateManagerInterface {

  sdl2w::Window* window;
  model::ItemInstance item;

public:
  ObserverShowLayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item)
      : window(_window), item(item) {}

  void onClick(int mouseX, int mouseY, int button) override;
};
} // namespace ui

} // export

namespace ui {

ListPickUp::ListPickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListPickUp::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListPickUp::setProps(const ListPickUpProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListPickUpProps& ListPickUp::getProps() const { return props; }

bmin::String ListPickUp::shortcutLetterForIndex(int index) {
  if (index < 0 || index >= maxShortcutItems) {
    return {};
  }
  return bmin::String(1, static_cast<char>('a' + index));
}

UiElement* ListPickUp::createItemElement(const ListPickUpPropsItem& listItem,
                                         int index) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(listItem.item.itemTemplateName);
  container->setScale(1.0f);
  container->setProps(QuadProps{
      .width = rowWidth,
      .height = rowHeight,
      .bgColor = Colors::Transparent,
  });

  const int iconWidth = static_cast<int>(iconSpriteSize * iconScale * style.scale);
  auto icon = new Quad(window, this);
  icon->setId("icon");
  icon->setPos(0, (rowHeight - iconWidth) / 2);
  icon->setScale(iconScale * style.scale);
  icon->setProps(QuadProps{
      .width = iconSpriteSize,
      .height = iconSpriteSize,
      .bgColor = {66, 202, 253, 50},
      .bgSprite = listItem.itemSprite,
  });
  container->addChild(icon);

  const int weightRightPadding = 8;
  const int labelWeightGap = 8;
  const int scaledContextBtnSize = contextBtnSize * style.scale;

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  int labelX =
      iconWidth + static_cast<int>(labelGapAfterIconNoShortcut * style.scale);
  const auto shortcutLetter = shortcutLetterForIndex(index);
  if (!shortcutLetter.empty()) {
    auto shortcutText = new TextLine(window, this);
    shortcutText->setId("shortcut");
    shortcutText->setPos(0, rowHeight / 2);
    shortcutText->setScale(1.0f);
    TextLineProps shortcutProps;
    shortcutProps.fontFamily = font.fontFamily;
    shortcutProps.fontSize = sdl2w::TEXT_SIZE_14;
    shortcutProps.fontColor = Colors::Grey;
    shortcutProps.textAlign = TextAlign::LEFT_CENTER;
    shortcutProps.textBlocks.pushBack({.text = shortcutLetter});
    shortcutText->setProps(shortcutProps);
    const int shortcutX =
        iconWidth + static_cast<int>(shortcutGapAfterIcon * style.scale);
    shortcutText->setPos(shortcutX, rowHeight / 2);
    container->addChild(shortcutText);
    labelX = shortcutX + shortcutText->getDims().first +
             static_cast<int>(labelGapAfterShortcut * style.scale);
  }

  auto contextBtn = new ButtonModal(window, this);
  contextBtn->setId("contextBtn");
  contextBtn->setPos(rowWidth - scaledContextBtnSize,
                     (rowHeight - scaledContextBtnSize) / 2);
  contextBtn->setScale(1.0f);
  contextBtn->setProps(ButtonModalProps{
      .text = "?",
      .width = scaledContextBtnSize,
      .height = scaledContextBtnSize,
      .bgColor = Colors::Transparent,
      .bgColorTopRight = Colors::Transparent,
      .bgColorBottomLeft = Colors::Transparent,
      .fontColor = Colors::DarkBlue,
  });
  contextBtn->addEventObserver(
      new ui::ObserverShowLayerPickUpContext(window, listItem.item));
  container->addChild(contextBtn);

  auto weightText = new TextLine(window, this);
  weightText->setId("weightText");
  weightText->setPos(0, rowHeight / 2);
  weightText->setScale(1.0f);
  TextLineProps weightProps;
  weightProps.fontFamily = font.fontFamily;
  weightProps.fontSize = font.fontSize;
  weightProps.fontColor = Colors::DarkGrey;
  weightProps.textAlign = TextAlign::LEFT_CENTER;
  weightProps.textBlocks.pushBack({
      .text = bmin::toString(listItem.item.quantity * listItem.weight) + " lbs",
  });
  weightText->setProps(weightProps);
  auto [weightWidth, _] = weightText->getDims();
  const int weightX = rowWidth - scaledContextBtnSize - weightWidth - weightRightPadding;
  weightText->setPos(weightX, rowHeight / 2);
  container->addChild(weightText);

  const int labelWidth = static_cast<int>(weightX - labelX - labelWeightGap);
  auto label = new ButtonTextWrap(window, this);
  label->setId("label");
  label->setPos(labelX, 0);
  label->setScale(1.0f);
  label->setProps(ButtonTextWrapProps{
      .textParagraph =
          {
              .textBlocks = {{.text = listItem.item.itemTemplateName}},
              .width = labelWidth,
              .fontFamily = font.fontFamily,
              .fontSize = sdl2w::TEXT_SIZE_18,
              .fontColor = Colors::Black,
          },
  });
  const int textOnlyHeight = label->getDims().second;
  const int verticalPadding =
      std::max(0, static_cast<int>((rowHeight - textOnlyHeight) / 2));
  label->setProps(ButtonTextWrapProps{
      .verticalPadding = verticalPadding,
      .textParagraph =
          {
              .textBlocks = {{.text = listItem.itemLabel}},
              .width = labelWidth,
              .fontFamily = font.fontFamily,
              .fontSize = sdl2w::TEXT_SIZE_18,
              .fontColor = Colors::Black,
          },
  });
  label->addEventObserver(new ui::ObserverPickUpItem(listItem.item));
  container->addChild(label);

  return container;
}

void ListPickUp::build() {
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

  for (size_t i = 0; i < props.items.size(); i++) {
    list->addChild(createItemElement(props.items[i], static_cast<int>(i)));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListPickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverPickUpItem::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    LOG(INFO) << "ObserverPickUpItem::onClick id=" << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(),
                                state::actions::pickUpItem(itemId),
                                0);
  }

void ObserverShowLayerPickUpContext::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverShowLayerPickUpContext::onClick " << item.itemTemplateName << " "
              << item.id << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::showLayerPickupContext(window, item),
        0);
  }

} // namespace ui

export {

// --- from ui/components/ConfirmModal.h ---
namespace ui {


struct ConfirmModalProps {
  bmin::String title = bmin::String(TRANSLATE("Confirm"));
  bmin::String message;
  bmin::String confirmButtonLabel = bmin::String(TRANSLATE("Yes"));
  bmin::String cancelButtonLabel = bmin::String(TRANSLATE("No"));
};

class ConfirmModal : public UiElement {
  ConfirmModalProps props;

  ButtonGroup* buttonGroup = nullptr;

public:
  ConfirmModal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ConfirmModal() override = default;

  void setProps(const ConfirmModalProps& _props);
  ConfirmModalProps& getProps();
  const ConfirmModalProps& getProps() const;

  ButtonGroup* getButtonGroup();
  const ButtonGroup* getButtonGroup() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ConfirmModal::ConfirmModal(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  style.width = 320;
}

void ConfirmModal::setProps(const ConfirmModalProps& _props) {
  props = _props;
  build();
}

ConfirmModalProps& ConfirmModal::getProps() { return props; }

const ConfirmModalProps& ConfirmModal::getProps() const { return props; }

ButtonGroup* ConfirmModal::getButtonGroup() { return buttonGroup; }

const ButtonGroup* ConfirmModal::getButtonGroup() const { return buttonGroup; }

void ConfirmModal::build() {
  children.clear();
  buttonGroup = nullptr;

  const int padding = 8;
  const int paddingScaled = static_cast<int>(padding * style.scale);
  const int contentWidth = style.width - 2 * padding;

  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);

  auto title = new TextLine(window, this);
  title->setId("title");
  title->setPos(style.x + paddingScaled, style.y + paddingScaled);
  title->setScale(1.f);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  titleProps.textBlocks.pushBack({.text = props.title});
  title->setProps(titleProps);
  auto [titleWidth, titleHeight] = title->calculateTextDims();
  addChild(title);

  const int messageY = style.y + paddingScaled + titleHeight + paddingScaled;

  TextFontProps messageFont;
  setBaseFontConfig(messageFont, BaseFontConfig::MODAL_TEXT);

  auto message = new TextParagraph(window, this);
  message->setId("message");
  message->setPos(style.x + paddingScaled, messageY);
  message->setScale(1.f);
  TextParagraphProps messageProps;
  messageProps.width = contentWidth;
  messageProps.fontFamily = messageFont.fontFamily;
  messageProps.fontSize = messageFont.fontSize;
  messageProps.fontColor = Colors::Black;
  messageProps.textAlign = TextAlign::LEFT_TOP;
  messageProps.lineSpacing = 0;
  messageProps.textBlocks.pushBack({.text = props.message});
  message->setProps(messageProps);
  auto [_, messageHeight] = message->getDims();
  addChild(message);

  const int buttonsY = messageY + messageHeight + paddingScaled;

  buttonGroup = new ButtonGroup(window, this);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(style.x + paddingScaled, buttonsY);
  buttonGroup->setScale(style.scale);
  ButtonGroupProps groupProps;
  groupProps.width = contentWidth;
  groupProps.alignment = ButtonGroupAlignment::RIGHT;
  groupProps.buttons.pushBack({.label = props.cancelButtonLabel});
  groupProps.buttons.pushBack({.label = props.confirmButtonLabel});
  buttonGroup->setProps(groupProps);
  auto [__, buttonGroupHeight] = buttonGroup->getDims();
  addChild(buttonGroup);

  style.height = ((buttonsY + buttonGroupHeight + paddingScaled) - style.y) / style.scale;

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  children.insert(children.begin(), bmin::UniquePtr<UiElement>(border));
}

void ConfirmModal::render(int dt) { UiElement::render(dt); }

} // namespace ui

export {

// --- from ui/components/FloatingNotification.h ---
namespace ui {

struct FloatingNotificationProps {
  bmin::String id;
  bmin::String message;
  state::UiFloatingNotificationType type = state::UiFloatingNotificationType::INFO;
};

class FloatingNotification : public UiElement {
private:
  static constexpr int kHorizontalPadding = 16;
  static constexpr int kVerticalPadding = 8;

  FloatingNotificationProps props;

  SDL_Color getTextColor() const;

public:
  FloatingNotification(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotification() override = default;

  void setProps(const FloatingNotificationProps& _props);
  const FloatingNotificationProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

FloatingNotification::FloatingNotification(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = false;
}

SDL_Color FloatingNotification::getTextColor() const {
  switch (props.type) {
  case state::UiFloatingNotificationType::WARNING:
    return Colors::Black;
  case state::UiFloatingNotificationType::ERROR:
    return Colors::Red;
  case state::UiFloatingNotificationType::INFO:
  default:
    return Colors::Blue;
  }
}

void FloatingNotification::setProps(const FloatingNotificationProps& _props) {
  props = _props;
  build();
}

const FloatingNotificationProps& FloatingNotification::getProps() const { return props; }

const std::pair<int, int> FloatingNotification::getDims() const {
  return {style.width, style.height};
}

void FloatingNotification::build() {
  children.clear();

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  auto textLine = new TextLine(window, this);
  textLine->setPos(style.x, style.y);
  textLine->setScale(1.f);
  TextLineProps notificationProps;
  notificationProps.fontFamily = font.fontFamily;
  notificationProps.fontSize = font.fontSize;
  notificationProps.fontColor = getTextColor();
  notificationProps.textAlign = TextAlign::CENTER;
  notificationProps.textBlocks.pushBack(
      {.text = props.message, .fontColor = getTextColor()});
  textLine->setProps(notificationProps);

  auto [textWidth, textHeight] = textLine->calculateTextDims();
  const int contentWidth = textWidth + kHorizontalPadding * 2;
  const int contentHeight = textHeight + kVerticalPadding * 2;

  style.width = contentWidth;
  style.height = contentHeight;

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setProps(BorderDropShadowProps{
      .width = contentWidth,
      .height = contentHeight,
      .backgroundColor = Colors::White,
      .shadowColor = Colors::Black,
      .shadowOffsetX = -4,
      .shadowOffsetY = 4,
      .borderSize = 2,
  });

  // TextLine renders in screen space (sibling of border), not inside the panel Quad.
  textLine->setPos(style.x + contentWidth / 2, style.y + contentHeight / 2);

  addChild(border);
  addChild(textLine);
}

void FloatingNotification::render(int dt) { UiElement::render(dt); }

} // namespace ui

export {

// --- from ui/components/borders/BorderInGameNarrow.h ---
namespace ui {

struct BorderInGameNarrowProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int partyMemberAreaHeight = 72;
  int sideBorderWidth = 16;
};

// BorderInGameNarrow component - renders a narrow in-game border layout
class BorderInGameNarrow : public BorderInGame {
private:
  BorderInGameNarrowProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameNarrow() override = default;

  void setProps(const BorderInGameNarrowProps& _props);
  BorderInGameNarrowProps& getProps();
  const BorderInGameNarrowProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui

} // export

namespace ui {

BorderInGameNarrow::BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent)
    : BorderInGame(_window, _parent) {}

void BorderInGameNarrow::setProps(const BorderInGameNarrowProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

BorderInGameNarrowProps& BorderInGameNarrow::getProps() { return props; }

const BorderInGameNarrowProps& BorderInGameNarrow::getProps() const { return props; }

const std::pair<int, int> BorderInGameNarrow::getContentAreaLocation() const {
  int contentX = style.x + props.sideBorderWidth * style.scale;
  int contentY = style.y + props.titleHeight * style.scale +
                 props.partyMemberAreaHeight * style.scale;
  return {contentX, contentY};
}

const std::pair<int, int> BorderInGameNarrow::getContentDims() const {
  return {(style.width - props.sideBorderWidth - props.sideBorderWidth) * style.scale,
          (style.height - props.titleHeight - props.partyMemberAreaHeight -
           ACTION_BUTTON_SIZE * props.actionButtonsScale - props.outsetBorderSize * 2) *
              style.scale};
}

const std::pair<int, int> BorderInGameNarrow::getPartyMemberAreaLocation() const {
  int partyX = style.x + props.outsetBorderSize * style.scale;
  auto [titleX, titleY] = getTitleLocation();
  auto [titleWidth, titleHeight] = getTitleDims();
  int partyY = titleY + titleHeight + props.outsetBorderSize * 2.f * style.scale;
  return {partyX, partyY};
}

const std::pair<int, int> BorderInGameNarrow::getActionButtonsAreaLocation() const {
  int actionX = style.x + props.sideBorderWidth * style.scale +
                props.outsetBorderSize * style.scale;
  int actionY = style.y + style.height * style.scale -
                ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale +
                -props.outsetBorderSize * 2 * style.scale +
                props.outsetBorderSize * style.scale;
  return {actionX, actionY};
}

void BorderInGameNarrow::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  int width = scaledWidth();
  int height = scaledHeight();
  int sideBorderY = style.y + props.titleHeight * style.scale +
                    props.partyMemberAreaHeight * style.scale;
  int sideBorderHeightScaled = height - props.titleHeight * style.scale -
                               props.partyMemberAreaHeight * style.scale;

  addOutsetRect(style.x, style.y, width / style.scale, props.titleHeight);

  addOutsetRect(style.x,
                style.y + props.titleHeight * style.scale,
                width / style.scale,
                props.partyMemberAreaHeight);

  addOutsetRect(style.x + width - props.sideBorderWidth * style.scale,
                sideBorderY,
                props.sideBorderWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x,
                sideBorderY,
                props.sideBorderWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x + props.sideBorderWidth * style.scale,
                style.y + height -
                    ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                    props.outsetBorderSize * 2 * style.scale,
                width / style.scale - props.sideBorderWidth * 2,
                ACTION_BUTTON_SIZE * props.actionButtonsScale + props.outsetBorderSize * 2);
}

} // namespace ui

export {

// --- from ui/components/borders/BorderInGameWide.h ---
namespace ui {

struct BorderInGameWideProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int subtitleHeight = 24;
  int partyMemberAreaWidth = 76;
  int leftBorderWidth = 16;
};

// BorderInGameWide component - renders a wide in-game border layout
class BorderInGameWide : public BorderInGame {
private:
  BorderInGameWideProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameWide(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameWide() override = default;

  void setProps(const BorderInGameWideProps& _props);
  BorderInGameWideProps& getProps();
  const BorderInGameWideProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui

} // export

namespace ui {

BorderInGameWide::BorderInGameWide(sdl2w::Window* _window, UiElement* _parent)
    : BorderInGame(_window, _parent) {}

void BorderInGameWide::setProps(const BorderInGameWideProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

BorderInGameWideProps& BorderInGameWide::getProps() { return props; }

const BorderInGameWideProps& BorderInGameWide::getProps() const { return props; }

const std::pair<int, int> BorderInGameWide::getContentAreaLocation() const {
  int contentX = style.x + props.leftBorderWidth * style.scale;
  int contentY = style.y + props.titleHeight * style.scale;
  return {contentX, contentY};
}

const std::pair<int, int> BorderInGameWide::getContentDims() const {
  return {(style.width - props.leftBorderWidth - props.partyMemberAreaWidth) *
              style.scale,
          (style.height - props.titleHeight -
           ACTION_BUTTON_SIZE * props.actionButtonsScale - props.outsetBorderSize * 2) *
              style.scale};
}

const std::pair<int, int> BorderInGameWide::getPartyMemberAreaLocation() const {
  int partyX = style.x + style.width * style.scale -
               props.partyMemberAreaWidth * style.scale +
               props.outsetBorderSize * style.scale;
  int partyY =
      style.y + props.titleHeight * style.scale + props.outsetBorderSize * style.scale;
  return {partyX, partyY};
}

const std::pair<int, int> BorderInGameWide::getActionButtonsAreaLocation() const {
  int actionX = style.x + props.leftBorderWidth * style.scale +
                props.outsetBorderSize * style.scale;
  int actionY = style.y + style.height * style.scale -
                ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                props.outsetBorderSize * 2 * style.scale +
                props.outsetBorderSize * style.scale;
  return {actionX, actionY};
}

void BorderInGameWide::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  int width = scaledWidth();
  int height = scaledHeight();
  int sideBorderY = style.y + props.titleHeight * style.scale;
  int sideBorderHeightScaled = height - props.titleHeight * style.scale;

  addOutsetRect(style.x, style.y, width / style.scale, props.titleHeight);

  addOutsetRect(style.x + width - props.partyMemberAreaWidth * style.scale,
                sideBorderY,
                props.partyMemberAreaWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x,
                sideBorderY,
                props.leftBorderWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x + props.leftBorderWidth * style.scale,
                style.y + height -
                    ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                    props.outsetBorderSize * 2 * style.scale,
                width / style.scale - props.leftBorderWidth - props.partyMemberAreaWidth,
                ACTION_BUTTON_SIZE * props.actionButtonsScale + props.outsetBorderSize * 2);
}

} // namespace ui

export {

// --- from ui/components/borders/BorderModalSmall.h ---
namespace ui {

struct BorderModalSmallProps {
  int width = 0;
  int height = 0;
  int headerHeight = 80;
  int iconSize = 64;
  int borderWidth = 2;
};

class BorderModalSmall : public UiElement {
protected:
  BorderModalSmallProps props;

public:
  BorderModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalSmall() override = default;

  void setProps(const BorderModalSmallProps& _props);
  BorderModalSmallProps& getProps();
  const BorderModalSmallProps& getProps() const;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconBorderLocation() const;
  const std::pair<int, int> getIconLocationCenter() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void buildTiledOverlay();
  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

BorderModalSmall::BorderModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void BorderModalSmall::setProps(const BorderModalSmallProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

BorderModalSmallProps& BorderModalSmall::getProps() { return props; }

const BorderModalSmallProps& BorderModalSmall::getProps() const { return props; }

const std::pair<int, int> BorderModalSmall::getDims() const {
  return {style.width * style.scale, style.height * style.scale};
}

const std::pair<int, int> BorderModalSmall::getContentDims() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {scaledWidth - scaledBorderWidth * 2,
          scaledHeight - scaledBorderWidth * 2 - scaledHeaderHeight};
}

const std::pair<int, int> BorderModalSmall::getContentLocation() const {
  int contentX = style.x + props.borderWidth * style.scale;
  int contentY =
      style.y + props.headerHeight * style.scale + props.borderWidth * style.scale;
  return {contentX, contentY};
}

const std::pair<int, int> BorderModalSmall::getIconBorderLocation() const {
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int margin = style.scale * (props.headerHeight - props.iconSize) / 2;
  int iconBorderX = style.x + scaledBorderWidth + margin;
  int iconBorderY = style.y + scaledBorderWidth + margin;
  return {iconBorderX, iconBorderY};
}

const std::pair<int, int> BorderModalSmall::getIconLocationCenter() const {
  auto [iconBorderX, iconBorderY] = getIconBorderLocation();
  return {iconBorderX + props.iconSize * style.scale / 2,
          iconBorderY + props.iconSize * style.scale / 2};
}

const std::pair<int, int> BorderModalSmall::getCloseButtonLocation() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  return {style.x + scaledWidth - scaledBorderWidth - 32 * style.scale,
          style.y + scaledBorderWidth};
}

const std::pair<int, int> BorderModalSmall::getTitleLocation() const {
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int margin = style.scale * (props.headerHeight - props.iconSize) / 2;
  int titleX = style.x + scaledBorderWidth + margin * 2 * style.scale +
               props.iconSize * style.scale;
  int titleY = style.y + scaledBorderWidth + props.headerHeight * style.scale / 2;
  return {titleX, titleY};
}

void BorderModalSmall::buildTiledOverlay() {
  removeChildById("tiledOverlay");
  if (style.width <= 0 || style.height <= 0) {
    return;
  }

  auto* overlay = new TiledOverlay(window, this);
  overlay->setId("tiledOverlay");
  overlay->setPos(style.x, style.y);
  overlay->setScale(style.scale);
  overlay->setProps(TiledOverlayProps{
      .width = style.width,
      .height = style.height,
      .spriteName = "ui_overlay_256",
      .alpha = 40,
  });
  addChild(overlay);
}

void BorderModalSmall::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  buildTiledOverlay();
}

void BorderModalSmall::render(int dt) {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  auto& draw = window->getDraw();
  // border
  draw.drawRect(
      style.x, style.y, scaledWidth, scaledHeight, Colors::BorderModalStandardDark);
  // background
  draw.drawRect(style.x + scaledBorderWidth,
                style.y + scaledBorderWidth,
                scaledWidth - scaledBorderWidth * 2,
                scaledHeight - scaledBorderWidth * 2,
                Colors::ModalStandardBackground);
  // title background
  draw.drawRect(style.x + scaledBorderWidth,
                style.y + scaledBorderWidth,
                scaledWidth - scaledBorderWidth * 2,
                props.headerHeight * style.scale,
                Colors::ModalHeaderBackground);
  // icon background
  auto [iconBorderX, iconBorderY] = getIconBorderLocation();
  draw.drawRect(iconBorderX,
                iconBorderY,
                props.iconSize * style.scale,
                props.iconSize * style.scale,
                Colors::DarkBlue);
  UiElement::render(dt);
}

} // namespace ui

export {

// --- from ui/components/FloatingNotificationSection.h ---
namespace ui {

struct FloatingNotificationSectionProps {
  int bottomMargin = 40;
  int notificationGap = 8;
};

class FloatingNotificationSection : public UiElement {
private:
  FloatingNotificationSectionProps props;
  std::uint64_t syncedNotificationRevision =
      std::numeric_limits<std::uint64_t>::max();

  template <state::ActionEvent Event, typename Fn> void subscribeAction(Fn&& fn) {
    if (!hasStateManager()) {
      return;
    }
    getStateManager()->getActionBus().subscribe(
        this,
        Event,
        [fn = std::forward<Fn>(fn)](state::AbstractAction& action, state::State& state) {
          fn(action, state);
        });
  }

  void syncFromState(const state::State& state);
  void layoutNotifications();

public:
  FloatingNotificationSection(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotificationSection() override;

  void setProps(const FloatingNotificationSectionProps& _props);
  const FloatingNotificationSectionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

FloatingNotificationSection::FloatingNotificationSection(sdl2w::Window* _window,
                                                           UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = false;

  subscribeAction<state::ActionEvent::UiPushFloatingNotification>(
      [this](const state::AbstractAction&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::ActionEvent::UiRemoveFloatingNotification>(
      [this](const state::AbstractAction&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::ActionEvent::UiToggleEquipInventoryItem>(
      [this](const state::AbstractAction&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::ActionEvent::UiGiveInventoryItem>(
      [this](const state::AbstractAction&, const state::State& state) {
        syncFromState(state);
      });

  if (hasStateManager()) {
    syncFromState(getStateManager()->getState());
  }
}

FloatingNotificationSection::~FloatingNotificationSection() {
  if (hasStateManager()) {
    getStateManager()->getActionBus().unsubscribe(this);
  }
}

void FloatingNotificationSection::setProps(const FloatingNotificationSectionProps& _props) {
  props = _props;
  build();
}

const FloatingNotificationSectionProps& FloatingNotificationSection::getProps() const {
  return props;
}

void FloatingNotificationSection::syncFromState(const state::State& state) {
  syncedNotificationRevision = state.uiState.floatingNotificationRevision;
  children.clear();

  for (const auto& notification : state.uiState.floatingNotifications) {
    auto floatingNotification = new FloatingNotification(window, this);
    floatingNotification->setId("notification-" + notification.id);
    floatingNotification->setProps(FloatingNotificationProps{
        .id = notification.id,
        .message = notification.message,
        .type = notification.type,
    });
    addChild(floatingNotification);
  }

  layoutNotifications();
}

void FloatingNotificationSection::layoutNotifications() {
  auto [windowWidth, windowHeight] = window->getDims();

  int stackHeight = 0;
  int maxWidth = 0;
  for (const auto& child : children) {
    if (stackHeight > 0) {
      stackHeight += props.notificationGap;
    }
    auto [childWidth, childHeight] = child->getDims();
    maxWidth = std::max(maxWidth, childWidth);
    stackHeight += childHeight;
  }

  // Anchor the stack at the bottom of the screen; newest notification sits lowest.
  int currentY = windowHeight - props.bottomMargin;
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto& child = *it;
    auto [childWidth, childHeight] = child->getDims();

    currentY -= childHeight;
    child->setPos((windowWidth - childWidth) / 2, currentY);
    child->build();

    currentY -= props.notificationGap;
  }

  style.x = (windowWidth - maxWidth) / 2;
  style.y = windowHeight - props.bottomMargin - stackHeight;
  style.width = maxWidth;
  style.height = stackHeight;
}

void FloatingNotificationSection::build() {
  if (hasStateManager()) {
    syncFromState(getStateManager()->getState());
  } else {
    layoutNotifications();
  }
}

void FloatingNotificationSection::render(int dt) {
  if (hasStateManager()) {
    const auto& state = getStateManager()->getState();
    if (syncedNotificationRevision != state.uiState.floatingNotificationRevision) {
      syncFromState(state);
    }
  }
  UiElement::render(dt);
}

} // namespace ui

export {

// --- from ui/components/borders/BorderModalStandard.h ---
namespace ui {

// struct BorderModalStandardProps {
//   int topLeftSquareSize = 78;
//   int leftBorderWidth = 16;
//   int borderSize = 4;
//   int closeButtonPadding = 6;
//   int subtitleYOffset = 40;
// };

// BorderModalStandard component - renders a specific border layout using OutsetRectangle
// elements Uses Position, Size, Scale from BaseStyle
class BorderModalStandard : public BorderModalSmall {
private:
  // BorderModalStandardProps props;

public:
  static const int BOTTOM_BORDER_HEIGHT = 10;
  BorderModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalStandard() override = default;

  // void setProps(const BorderModalStandardProps& _props);
  // BorderModalStandardProps& getProps();
  // const BorderModalStandardProps& getProps() const;

  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconSectionCenter() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getSubTitleLocation() const;
  const std::pair<int, int> getSubTitleDims() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

BorderModalStandard::BorderModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : BorderModalSmall(_window, _parent) {}

const std::pair<int, int> BorderModalStandard::getContentDims() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {scaledWidth - scaledBorderWidth * 2,
          scaledHeight - scaledBorderWidth * 2 - scaledHeaderHeight -
              BOTTOM_BORDER_HEIGHT * style.scale};
}

const std::pair<int, int> BorderModalStandard::getContentLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {style.x + scaledBorder, style.y + scaledBorder + scaledHeaderHeight};
}

const std::pair<int, int> BorderModalStandard::getIconSectionCenter() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  const int sectionSize = static_cast<int>(props.headerHeight * style.scale);
  const int centerX = style.x + scaledBorder + sectionSize / 2;
  const int centerY = style.y + scaledBorder + sectionSize / 2;
  return {centerX, centerY};
}

const std::pair<int, int> BorderModalStandard::getTitleLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto iconAreaSizeScaled = (props.headerHeight + 4 + 4) * style.scale;
  auto locX = style.x + scaledBorder + iconAreaSizeScaled;
  auto locY = style.y + scaledBorder +
              (static_cast<float>(props.headerHeight) / 4.f) * style.scale;
  return {locX, locY};
}

const std::pair<int, int> BorderModalStandard::getSubTitleLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto iconAreaSizeScaled = (props.headerHeight + 4 + 4) * style.scale;
  auto locX = style.x + scaledBorder + iconAreaSizeScaled;
  auto locY = style.y + scaledBorder +
              (static_cast<float>(props.headerHeight) * 3.f / 4.f) * style.scale;
  return {locX, locY};
}

const std::pair<int, int> BorderModalStandard::getSubTitleDims() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto iconAreaSizeScaled = (props.headerHeight + 4 + 4) * style.scale;
  return {style.width * style.scale - scaledBorder * 2 - iconAreaSizeScaled,
          (static_cast<float>(props.headerHeight) / 2.f) * style.scale};
}

const std::pair<int, int> BorderModalStandard::getCloseButtonLocation() const {
  auto [scaledWidth, scaledHeight] = getDims();
  OutsetRectangleProps outsetRectProps;
  int innerBorderSize = outsetRectProps.borderSize;
  int closeButtonSize = ButtonClose::closeButtonSize;
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  return {style.x + scaledWidth - scaledBorderWidth - closeButtonSize * style.scale -
              innerBorderSize * style.scale,
          style.y + scaledBorderWidth + innerBorderSize * style.scale};
}

void BorderModalStandard::build() {
  children.clear();

  auto [scaledWidth, scaledHeight] = getDims();
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);

  auto iconOutsetRect = new OutsetRectangle(window, this);
  iconOutsetRect->setPos(style.x + scaledBorder, style.y + scaledBorder);
  iconOutsetRect->setScale(style.scale);
  iconOutsetRect->setProps(OutsetRectangleProps{
      .width = props.headerHeight,
      .height = props.headerHeight,
  });
  iconOutsetRect->setId("iconOutsetRect");
  addChild(iconOutsetRect);

  auto topBarOutsetRect = new OutsetRectangle(window, this);
  topBarOutsetRect->setId("topBarOutsetRect");
  topBarOutsetRect->setPos(style.x + scaledBorder + props.headerHeight * style.scale,
                           style.y + scaledBorder);
  topBarOutsetRect->setScale(style.scale);
  topBarOutsetRect->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledWidth / style.scale - props.borderWidth * 2 -
                                props.headerHeight),
      .height = props.headerHeight / 2,
  });
  addChild(topBarOutsetRect);

  auto bottomBarOutsetRect = new OutsetRectangle(window, this);
  bottomBarOutsetRect->setId("bottomBarOutsetRect");
  bottomBarOutsetRect->setPos(
      style.x + scaledBorder,
      style.y + scaledHeight - scaledBorder - BOTTOM_BORDER_HEIGHT * style.scale);
  bottomBarOutsetRect->setScale(style.scale);
  bottomBarOutsetRect->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledWidth / style.scale - props.borderWidth * 2),
      .height = BOTTOM_BORDER_HEIGHT,
  });
  addChild(bottomBarOutsetRect);

  buildTiledOverlay();
}

void BorderModalStandard::render(int dt) { BorderModalSmall::render(dt); }

} // namespace ui

export {

// --- from ui/components/TouchMovePad.h ---
namespace ui {

struct TouchMovePadProps {
  int buttonGapH = 10;
  int buttonGapV = 12;
  int padding = 4;
  int borderSize = 4;
  int dragBarHeight = 18;
};

// TouchMovePad - floating directional pad for mouse/touch movement input.
class TouchMovePad : public UiElement {
  struct ButtonPlacement {
    MoveDirection direction;
    const char* id;
  };

  static constexpr int halfButtonW = 22;
  static constexpr int halfButtonH = 22;
  static constexpr int cardButtonW = 44;
  static constexpr int cardButtonH = 22;
  static constexpr int borderButtonW = 18;
  static constexpr int borderButtonH = 8;

  static constexpr ButtonPlacement buttonPlacements[] = {
      {MoveDirection::UpLeft, "move_up_left"},
      {MoveDirection::Up, "move_up"},
      {MoveDirection::UpRight, "move_up_right"},
      {MoveDirection::Left, "move_left"},
      {MoveDirection::Wait, "move_wait"},
      {MoveDirection::Right, "move_right"},
      {MoveDirection::DownLeft, "move_down_left"},
      {MoveDirection::Down, "move_down"},
      {MoveDirection::DownRight, "move_down_right"},
  };

  TouchMovePadProps props;
  bool isDragging = false;
  int dragOffsetX = 0;
  int dragOffsetY = 0;

  int getGridWidth() const;
  int getGridHeight() const;
  int getContentWidth() const;
  int getContentHeight() const;
  int getBodyY() const;
  int getBodyHeight() const;
  int getWideRowWidth() const;
  int getNarrowRowWidth() const;
  std::pair<int, int> getButtonPosition(MoveDirection direction) const;
  bool isInDragBar(int mouseX, int mouseY) const;
  void positionChildren();
  void syncDragHandle();
  void moveTo(int x, int y);

public:
  TouchMovePad(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TouchMovePad() override = default;

  void setProps(const TouchMovePadProps& _props);
  TouchMovePadProps& getProps();
  const TouchMovePadProps& getProps() const;

  void startDrag(int mouseX, int mouseY);
  void endDrag();

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

constexpr TouchMovePad::ButtonPlacement TouchMovePad::buttonPlacements[];

TouchMovePad::TouchMovePad(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void TouchMovePad::setProps(const TouchMovePadProps& _props) {
  props = _props;
  build();
}

TouchMovePadProps& TouchMovePad::getProps() { return props; }

const TouchMovePadProps& TouchMovePad::getProps() const { return props; }

int TouchMovePad::getWideRowWidth() const {
  return cardButtonW + props.buttonGapH + halfButtonW + props.buttonGapH + cardButtonW;
}

int TouchMovePad::getNarrowRowWidth() const {
  return halfButtonW + props.buttonGapH + cardButtonW + props.buttonGapH + halfButtonW;
}

int TouchMovePad::getGridWidth() const { return getWideRowWidth(); }

int TouchMovePad::getGridHeight() const {
  return halfButtonH + props.buttonGapV + halfButtonH + props.buttonGapV + halfButtonH;
}

int TouchMovePad::getContentWidth() const {
  return getGridWidth() + props.padding * 2 + props.borderSize * 2;
}

int TouchMovePad::getBodyHeight() const {
  return getGridHeight() + props.padding * 2 + props.borderSize * 2;
}

int TouchMovePad::getContentHeight() const {
  return props.dragBarHeight + getBodyHeight();
}

int TouchMovePad::getBodyY() const { return style.y + props.dragBarHeight; }

std::pair<int, int> TouchMovePad::getButtonPosition(MoveDirection direction) const {
  const int gridW = getWideRowWidth();
  const int narrowW = getNarrowRowWidth();
  const int rowInset = (gridW - narrowW) / 2;

  const int bodyY = style.y + props.dragBarHeight;
  const int gridX = style.x + props.borderSize + props.padding;
  const int gridY = bodyY + props.borderSize + props.padding;
  const int rowStep = halfButtonH + props.buttonGapV;

  int buttonX = gridX;
  int buttonY = gridY;

  switch (direction) {
  case MoveDirection::UpLeft:
    buttonX = gridX + rowInset;
    break;
  case MoveDirection::Up:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH;
    break;
  case MoveDirection::UpRight:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH + cardButtonW +
              props.buttonGapH;
    break;
  case MoveDirection::Left:
    buttonY = gridY + rowStep;
    break;
  case MoveDirection::Wait:
    buttonX = gridX + cardButtonW + props.buttonGapH;
    buttonY = gridY + rowStep;
    break;
  case MoveDirection::Right:
    buttonX = gridX + cardButtonW + props.buttonGapH + halfButtonW + props.buttonGapH;
    buttonY = gridY + rowStep;
    break;
  case MoveDirection::DownLeft:
    buttonX = gridX + rowInset;
    buttonY = gridY + rowStep * 2;
    break;
  case MoveDirection::Down:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH;
    buttonY = gridY + rowStep * 2;
    break;
  case MoveDirection::DownRight:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH + cardButtonW +
              props.buttonGapH;
    buttonY = gridY + rowStep * 2;
    break;
  }

  return {buttonX, buttonY};
}

bool TouchMovePad::isInDragBar(int mouseX, int mouseY) const {
  const int dragBarX = style.x;
  const int dragBarY = style.y;
  const int dragBarW = static_cast<int>(getContentWidth() * style.scale);
  const int dragBarH = static_cast<int>(props.dragBarHeight * style.scale);
  return isInBounds(mouseX, mouseY, dragBarX, dragBarY, dragBarW, dragBarH);
}

void TouchMovePad::moveTo(int x, int y) {
  setPos(x, y);
  positionChildren();
}

void TouchMovePad::positionChildren() {
  if (auto* background = getChildById("background")) {
    background->setPos(style.x, getBodyY());
  }

  if (auto* dragBar = getChildById("dragBar")) {
    dragBar->setPos(style.x, style.y);
  }

  if (auto* dragHandle = getChildById("dragHandle")) {
    dragHandle->setPos(style.x + (getContentWidth() - borderButtonW) / 2,
                       style.y + (props.dragBarHeight - borderButtonH) / 2);
  }

  for (const auto& placement : buttonPlacements) {
    if (auto* button = getChildById(placement.id)) {
      const auto [buttonX, buttonY] = getButtonPosition(placement.direction);
      button->setPos(buttonX, buttonY);
    }
  }
}

void TouchMovePad::syncDragHandle() {
  auto* dragHandle = getChildById("dragHandle");
  if (dragHandle == nullptr) {
    return;
  }

  if (auto* sprite = dynamic_cast<SpriteElement*>(dragHandle)) {
    sprite->setSprite(isDragging ? "ui_border_buttons_4" : "ui_border_buttons_0");
  }
}

void TouchMovePad::startDrag(int mouseX, int mouseY) {
  isDragging = true;
  dragOffsetX = mouseX - style.x;
  dragOffsetY = mouseY - style.y;
  syncDragHandle();
  positionChildren();
}

void TouchMovePad::endDrag() {
  if (!isDragging) {
    return;
  }
  isDragging = false;
  syncDragHandle();
  positionChildren();
}

bool TouchMovePad::checkMouseDownEvent(int mouseX,
                                       int mouseY,
                                       int button,
                                       bmin::DynArray<UiElement*> additionalElements) {
  if (isInDragBar(mouseX, mouseY)) {
    startDrag(mouseX, mouseY);
    return true;
  }
  return UiElement::checkMouseDownEvent(mouseX, mouseY, button, additionalElements);
}

bool TouchMovePad::checkMouseUpEvent(int mouseX,
                                     int mouseY,
                                     int button,
                                     bmin::DynArray<UiElement*> additionalElements) {
  endDrag();
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button, additionalElements);
}

bool TouchMovePad::checkHoverEvent(int mouseX,
                                   int mouseY,
                                   bmin::DynArray<UiElement*> additionalElements) {
  if (isDragging) {
    moveTo(mouseX - dragOffsetX, mouseY - dragOffsetY);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY, additionalElements);
}

void TouchMovePad::build() {
  children.clear();

  style.width = getContentWidth();
  style.height = getContentHeight();

  auto background = new OutsetRectangle(window, this);
  background->setId("background");
  background->setPos(style.x, getBodyY());
  background->setScale(style.scale);
  background->setProps(OutsetRectangleProps{
      .width = style.width,
      .height = getBodyHeight(),
      .color = Colors::BorderModalStandard,
      .colorTopRight = Colors::BorderModalStandardLight,
      .colorBottomLeft = Colors::BorderModalStandardDark,
      .borderSize = props.borderSize,
  });
  addChild(background);

  for (const auto& placement : buttonPlacements) {
    const auto [buttonX, buttonY] = getButtonPosition(placement.direction);

    auto button = new ButtonMove(window, this);
    button->setId(placement.id);
    button->setPos(buttonX, buttonY);
    button->setScale(style.scale);
    button->setProps(ButtonMoveProps{.direction = placement.direction});
    addChild(button);
  }

  auto dragBar = new OutsetRectangle(window, this);
  dragBar->setId("dragBar");
  dragBar->setPos(style.x, style.y);
  dragBar->setScale(style.scale);
  dragBar->setProps(OutsetRectangleProps{
      .width = style.width,
      .height = props.dragBarHeight,
      .color = Colors::BorderModalStandard,
      .colorTopRight = Colors::BorderModalStandardLight,
      .colorBottomLeft = Colors::BorderModalStandardDark,
      .borderSize = props.borderSize,
  });
  addChild(dragBar);

  auto dragHandle = new SpriteElement(window, this);
  dragHandle->setId("dragHandle");
  dragHandle->setPos(style.x + (style.width - borderButtonW) / 2,
                     style.y + (props.dragBarHeight - borderButtonH) / 2);
  dragHandle->setScale(style.scale);
  dragHandle->setProps(SpriteElementProps{
      .width = borderButtonW,
      .height = borderButtonH,
      .spriteName = "ui_border_buttons_0",
  });
  addChild(dragHandle);
}

void TouchMovePad::render(int dt) { UiElement::render(dt); }

} // namespace ui

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
