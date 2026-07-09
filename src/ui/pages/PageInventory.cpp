#include "PageInventory.h"
#include "layers/ui/LayerInventory.h"
#include "sdl2w/L10n.h"
#include "model/templates/Items.h"
#include "ui/colors.h"
#include "ui/components/PartyMemberIconSelector.h"
#include "ui/components/lists/ListInventory.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include <algorithm>

namespace ui {

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

void PageInventory::build() {
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
  if (!props.characterPlayerSprite.empty()) {
    modalProps.iconSprite = props.characterPlayerSprite;
  }
  modal->setProps(modalProps);
  addChild(modal);

  if (!props.characterPlayerSprite.empty()) {
    if (auto* icon = modal->getChildById("headerIcon")) {
      constexpr int kIconTextureSize = 32;
      constexpr float kIconScale = 2.f;
      auto [iconX, iconY] = icon->getPos();
      auto iconBg = bmin::makeUnique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setPos(iconX, iconY);
      iconBg->setScale(kIconScale * style.scale);
      iconBg->setProps(QuadProps{
          .width = kIconTextureSize,
          .height = kIconTextureSize,
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
        new ObserverRemoveLayer(layers::LayerInventory::LAYER_ID));
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
  titleBlock.text =
      bmin::String(TRANSLATE("Inventory")) + " - " + props.characterPlayerLabel;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto [subtitleX, subtitleY] = modal->getSubTitleLocation();

  const int partyIconSize = ButtonClose::closeButtonSize;
  const int scaledPartyIconSize = static_cast<int>(partyIconSize * style.scale);
  const int partyIconY = subtitleY - scaledPartyIconSize / 2;

  if (!props.partyMembers.empty()) {
    PartyMemberIconSelectorProps selectorProps;
    selectorProps.selectedIndex = props.partyMemberInventoryIndex;
    for (const auto& member : props.partyMembers) {
      selectorProps.members.pushBack(member.spriteName);
    }

    auto partySelector = new PartyMemberIconSelector(window, modal);
    partySelector->setId("partyMemberSelector");
    partySelector->setPos(subtitleX, partyIconY);
    partySelector->setScale(style.scale);
    partySelector->setProps(selectorProps);
    modal->addChild(partySelector);
  }

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
  modal->addChild(statsBar);

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
      .text = bmin::String(TRANSLATE("Carrying")) + " " + bmin::toString(props.weightCarrying) + "/" +
              bmin::toString(props.weightCapacity),
  });
  weightText->setScale(1.f);
  weightText->setProps(weightProps);
  weightText->setPos(contentX + statsRowPadding, statsRowY + scaledStatsRowHeight / 2);
  modal->addChild(weightText);

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
  modal->addChild(goldText);

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
  addChild(scrollableSection);

  // Create ListInventory inside the scrollable section
  auto listInventory = new ListInventory(window, scrollableSection);
  listInventory->setId("listInventory");
  listInventory->setPos(0, 0);
  listInventory->setScale(style.scale);

  ListInventoryProps listProps;
  listProps.characterPlayerId = props.characterPlayerId;
  listProps.width = static_cast<float>(contentW) / style.scale -
                    scrollableSection->getProps().scrollBarWidth - 8;
  populateInventoryProps(listProps.items);
  listInventory->setProps(listProps);
  scrollableSection->addChild(listInventory);
  scrollableSection->build();
}

void PageInventory::render(int dt) { UiElement::render(dt); }

} // namespace ui
