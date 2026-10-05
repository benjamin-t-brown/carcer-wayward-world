#include "PageInventory.h"
#include "sdl2w/L10n.h"
#include "model/templates/Items.h"
#include "ui/colors.hpp"
#include "ui/components/PartyMemberIconSelector.h"
#include "ui/components/lists/ListInventory.h"
#include "ui/components/lists/ListSpecialItems.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonSprite.h"
#include "ui/helpers/modalLayoutFit.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/observers/ActionObserver.hpp"
#include "actions/navigation/UiRemoveLayer.hpp"
#include <algorithm>

namespace ui {

class PageInventorySpecialItemsToggleObserver : public UiEventObserver {
  PageInventory* page;

public:
  explicit PageInventorySpecialItemsToggleObserver(PageInventory* _page) : page(_page) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (page) {
      page->toggleSpecialItemsView();
    }
  }
};

PageInventory::PageInventory(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageInventory::setProps(const PageInventoryProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageInventoryProps& PageInventory::getProps() { return props; }

const PageInventoryProps& PageInventory::getProps() const { return props; }

void PageInventory::populateInventoryProps(
    bmin::DynArray<ListInventoryPropsItem>& listProps) {
  if (!getStateManager() || !getDatabase()) {
    return;
  }
  auto& database = *getDatabase();

  model::CharacterPlayer equippedCheck;
  equippedCheck.equipment = props.equipment;

  for (const auto& item : props.inventory) {
    auto& itemTemplate = database.getItemTemplate(bmin::toStringView(item.itemName));
    const auto equippedSlot =
        model::characterPlayerGetEquipmentSlotForItemId(equippedCheck, item.id);
    bmin::String equippedSlotAbbrev;
    if (equippedSlot.has_value()) {
      equippedSlotAbbrev = model::characterEquipmentSlotAbbrev(*equippedSlot);
    }
    listProps.pushBack({.itemId = item.id,
                         .itemName = item.itemName,
                         .itemLabel = itemTemplate.label.empty() ? itemTemplate.name
                                                                 : itemTemplate.label,
                         .itemSprite = itemTemplate.iconSpriteName,
                         .isEquippable = model::itemTypeIsEquippable(itemTemplate.itemType),
                         .isEquipped = equippedSlot.has_value(),
                         .equippedSlotAbbrev = equippedSlotAbbrev,
                         .isStackable = itemTemplate.stackable,
                         .quantity = item.quantity});
  }
}

void PageInventory::populateSpecialItemsListProps(
    bmin::DynArray<ListSpecialItemsPropsItem>& listProps) {
  for (const auto& item : props.specialItems) {
    listProps.pushBack({.itemName = item.itemName,
                         .itemLabel = item.itemLabel,
                         .itemSprite = item.itemSprite,
                         .quantity = item.quantity});
  }
}

void PageInventory::toggleSpecialItemsView() {
  showingSpecialItems = !showingSpecialItems;
  // ButtonSprite already plays the click sound; do not rebuild here — clearing
  // children would destroy this button's observers before onMouseUp finishes.
  specialItemsNeedsRebuild = true;
}

void PageInventory::selectSpecialItemsView() {
  if (showingSpecialItems) {
    return;
  }
  showingSpecialItems = true;
  build();
}

void PageInventory::exitSpecialItemsView() {
  showingSpecialItems = false;
  specialItemsNeedsRebuild = false;
}

void PageInventory::build() {
  specialItemsNeedsRebuild = false;
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  ModalStandardProps modalProps;
  modalProps.width = style.width;
  modalProps.height = style.height;
  modalProps.portraitScale = props.portraitScale;
  if (!props.characterPlayerSprite.empty()) {
    modalProps.iconSprite = props.characterPlayerSprite;
  }
  modal->setProps(modalProps);
  syncHostStyleToCappedCentered(style);
  addChild(bmin::UniquePtr<ui::UiElement>(modal));

  if (!props.characterPlayerSprite.empty()) {
    if (auto* icon = modal->getChildById("headerIcon")) {
      auto [iconX, iconY] = icon->getPos();
      auto [iconW, iconH] = icon->getDims();
      auto iconBg = bmin::makeUnique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setPos(iconX, iconY);
      iconBg->setScale(1.f);
      iconBg->setProps(QuadProps{
          .width = iconW,
          .height = iconH,
          .bgColor = {255, 255, 255, 50},
      });

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(modalChildren.begin(),
                                             modalChildren.end(),
                                             [](const bmin::UniquePtr<UiElement>& child) {
                                               return child->getId() == "headerIcon";
                                             });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
      }
    }
  }

  auto closeButton = modal->getCloseButtonElement();
  if (closeButton) {
    closeButton->addEventObserver(
        ui::makeActionObserver<state::actions::UiRemoveLayer>(
            state::LayerId::Inventory));
  }

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = contentW / style.scale;
  int unscaledContentH = contentH / style.scale;

  // Create title element
  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  if (showingSpecialItems) {
    titleBlock.text = TRANSLATE("Special Items");
  } else {
    titleBlock.text =
        bmin::String(TRANSLATE("Inventory")) + " - " + props.characterPlayerLabel;
  }
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(bmin::UniquePtr<ui::UiElement>(title));

  auto [subtitleX, subtitleY] = modal->getSubTitleLocation();

  const int partyIconSize = ButtonClose::closeButtonSize;
  const int scaledPartyIconSize = static_cast<int>(partyIconSize * style.scale);
  const int partyIconY = subtitleY - scaledPartyIconSize / 2;
  const int scaledPartyIconGap =
      static_cast<int>(specialItemsButtonGap * style.scale);

  int specialItemsButtonX = subtitleX;
  if (!props.partyMembers.empty()) {
    PartyMemberIconSelectorProps selectorProps;
    // Special items are party-wide — no member should appear selected.
    selectorProps.selectedIndex =
        showingSpecialItems ? -1 : props.partyMemberInventoryIndex;
    for (const auto& member : props.partyMembers) {
      selectorProps.members.pushBack(member.spriteName);
    }

    auto partySelector = new PartyMemberIconSelector(window, modal);
    partySelector->setId("partyMemberSelector");
    partySelector->setPos(subtitleX, partyIconY);
    partySelector->setScale(style.scale);
    partySelector->setProps(selectorProps);
    // Same spacing as between party member icons.
    specialItemsButtonX =
        subtitleX + partySelector->getDims().first + scaledPartyIconGap;
    modal->addChild(bmin::UniquePtr<ui::UiElement>(partySelector));
  }

  const int specialButtonSpriteSize = 16;
  const int specialButtonPadding =
      (specialItemsButtonSize - specialButtonSpriteSize) / 2;
  auto specialItemsButton = new ButtonSprite(window, modal);
  specialItemsButton->setId("specialItemsButton");
  specialItemsButton->setPos(specialItemsButtonX, partyIconY);
  specialItemsButton->setScale(style.scale);
  specialItemsButton->setProps(ButtonSpriteProps{
      .spriteName = specialItemsButtonSprite,
      .spriteWidth = specialButtonSpriteSize,
      .spriteHeight = specialButtonSpriteSize,
      .padding = specialButtonPadding,
      .isSelected = showingSpecialItems,
  });
  specialItemsButton->addEventObserver(bmin::UniquePtr<UiEventObserver>(
      new PageInventorySpecialItemsToggleObserver(this)));
  modal->addChild(bmin::UniquePtr<ui::UiElement>(specialItemsButton));

  const int statsRowHeight = 32;
  const int scaledStatsRowHeight = static_cast<int>(statsRowHeight * style.scale);
  const int statsRowPadding = static_cast<int>(16 * style.scale);
  const int statsRowY = contentY;
  const int scrollableY = contentY + scaledStatsRowHeight;
  const int scrollableHeight = unscaledContentH - statsRowHeight;

  auto statsBar = new Quad(window, modal);
  statsBar->setId("statsBar");
  statsBar->setPos(contentX, statsRowY);
  statsBar->setScale(style.scale);
  statsBar->setProps(QuadProps{
      .width = unscaledContentW,
      .height = statsRowHeight,
      .bgColor = Colors::White,
  });
  modal->addChild(bmin::UniquePtr<ui::UiElement>(statsBar));

  if (!showingSpecialItems) {
    auto weightText = new TextLine(window, modal);
    weightText->setId("statsWeight");
    TextFontProps weightFont;
    setBaseFontConfig(weightFont, BaseFontConfig::MODAL_TEXT);
    TextLineProps weightProps;
    weightProps.fontFamily = weightFont.fontFamily;
    weightProps.fontSize = weightFont.fontSize;
    weightProps.fontColor = Colors::DarkGrey;
    weightProps.textAlign = TextAlign::LEFT_CENTER;
    weightProps.textBlocks.pushBack({
        .text = bmin::String(TRANSLATE("Carrying")) + " " +
                bmin::toString(props.weightCarrying) + "/" +
                bmin::toString(props.weightCapacity),
    });
    weightText->setScale(1.f);
    weightText->setProps(weightProps);
    weightText->setPos(contentX + statsRowPadding, statsRowY + scaledStatsRowHeight / 2);
    modal->addChild(bmin::UniquePtr<ui::UiElement>(weightText));
  }

  auto goldText = new TextLine(window, modal);
  goldText->setId("statsGold");
  TextFontProps goldFont;
  setBaseFontConfig(goldFont, BaseFontConfig::MODAL_TEXT);
  TextLineProps goldProps;
  goldProps.fontFamily = goldFont.fontFamily;
  goldProps.fontSize = goldFont.fontSize;
  goldProps.fontColor = Colors::DarkGrey;
  goldProps.textAlign = TextAlign::LEFT_CENTER;
  goldProps.textBlocks.pushBack({
      .text = bmin::toString(props.gold) + bmin::String(TRANSLATE(" gp")),
      .fontColor = Colors::Blue,
  });
  goldText->setScale(1.f);
  goldText->setProps(goldProps);
  goldText->setPos(contentX + contentW - goldText->getDims().first - statsRowPadding,
                   statsRowY + scaledStatsRowHeight / 2);
  modal->addChild(bmin::UniquePtr<ui::UiElement>(goldText));

  // Create SectionScrollable for content area
  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  scrollableSection->setPos(contentX, scrollableY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = scrollableHeight,
      .scrollBarWidth = 40,
  });
  addChild(bmin::UniquePtr<ui::UiElement>(scrollableSection));

  const int listWidth = static_cast<int>(static_cast<float>(contentW) / style.scale -
                                         scrollableSection->getProps().scrollBarWidth - 8);

  if (showingSpecialItems) {
    auto listSpecialItems = new ListSpecialItems(window, scrollableSection);
    listSpecialItems->setId("listSpecialItems");
    listSpecialItems->setPos(0, 0);
    listSpecialItems->setScale(style.scale);

    ListSpecialItemsProps listProps;
    listProps.width = listWidth;
    populateSpecialItemsListProps(listProps.items);
    listSpecialItems->setProps(listProps);
    scrollableSection->addChild(bmin::UniquePtr<ui::UiElement>(listSpecialItems));
  } else {
    auto listInventory = new ListInventory(window, scrollableSection);
    listInventory->setId("listInventory");
    listInventory->setPos(0, 0);
    listInventory->setScale(style.scale);

    ListInventoryProps listProps;
    listProps.characterPlayerId = props.characterPlayerId;
    listProps.width = listWidth;
    populateInventoryProps(listProps.items);
    listInventory->setProps(listProps);
    scrollableSection->addChild(bmin::UniquePtr<ui::UiElement>(listInventory));
  }
  scrollableSection->build();
}

void PageInventory::render(int dt) {
  if (specialItemsNeedsRebuild) {
    specialItemsNeedsRebuild = false;
    build();
  }
  UiElement::render(dt);
}

} // namespace ui
