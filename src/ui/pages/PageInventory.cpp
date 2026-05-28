#include "PageInventory.h"
#include "layers/ui/LayerInventory.h"
#include "lib/sdl2w/L10n.h"
#include "model/Items.h"
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
  build();
}

PageInventoryProps& PageInventory::getProps() { return props; }

const PageInventoryProps& PageInventory::getProps() const { return props; }

void PageInventory::populateInventoryProps(
    std::vector<ListInventoryPropsItem>& listProps) {
  if (!getStateManager() || !getDatabase()) {
    return;
  }
  auto& database = *getDatabase();

  model::CharacterPlayer equippedCheck;
  equippedCheck.equipment = props.equipment;

  for (const auto& item : props.inventory) {
    auto& itemTemplate = database.getItemTemplate(item.itemName);
    listProps.push_back({.itemId = item.id,
                         .itemName = item.itemName,
                         .itemLabel = itemTemplate.label.empty() ? itemTemplate.name
                                                                 : itemTemplate.label,
                         .itemSprite = itemTemplate.iconSpriteName,
                         .isEquippable = model::itemTypeIsEquippable(itemTemplate.itemType),
                         .isEquipped =
                             model::characterPlayerIsItemEquippedById(equippedCheck,
                                                                      item.id),
                         .isStackable = itemTemplate.stackable,
                         .quantity = item.quantity});
  }
}

void PageInventory::build() {
  children.clear();

  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  auto& modalStyle = modal->getStyle();
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  ModalStandardProps modalProps;
  if (!props.characterPlayerSprite.empty()) {
    modalProps.iconSprite = props.characterPlayerSprite;
  }
  modal->setProps(modalProps);
  addChild(modal);

  if (!props.characterPlayerSprite.empty()) {
    if (auto* icon = modal->getChildById("headerIcon")) {
      auto iconBg = std::make_unique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setStyle(icon->getStyle());
      iconBg->setProps(QuadProps{.bgColor = {255, 255, 255, 50}});

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(modalChildren.begin(),
                                             modalChildren.end(),
                                             [](const std::unique_ptr<UiElement>& child) {
                                               return child->getId() == "headerIcon";
                                             });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, std::move(iconBg));
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
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text =
      std::string(TRANSLATE("Inventory")) + " - " + props.characterPlayerLabel;
  titleProps.textBlocks.push_back(titleBlock);
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
      selectorProps.members.push_back(member.spriteName);
    }

    auto partySelector = new PartyMemberIconSelector(window, modal);
    partySelector->setId("partyMemberSelector");
    auto& selectorStyle = partySelector->getStyle();
    selectorStyle.x = subtitleX;
    selectorStyle.y = partyIconY;
    selectorStyle.scale = style.scale;
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
  auto& statsBarStyle = statsBar->getStyle();
  statsBarStyle.x = contentX;
  statsBarStyle.y = statsRowY;
  statsBarStyle.width = unscaledContentW;
  statsBarStyle.height = statsRowHeight;
  statsBarStyle.scale = style.scale;
  statsBar->setProps(QuadProps{.bgColor = Colors::White});
  modal->addChild(statsBar);

  auto weightText = new TextLine(window, modal);
  weightText->setId("statsWeight");
  auto& weightStyle = weightText->getStyle();
  setBaseFontConfig(weightStyle, BaseFontConfig::MODAL_TEXT);
  weightStyle.fontColor = Colors::DarkGrey;
  weightStyle.textAlign = TextAlign::LEFT_CENTER;
  weightStyle.scale = 1.f;
  weightText->setProps(TextLineProps{
      .textBlocks = {{
          .text = std::string(TRANSLATE("Carrying")) + " " +
                  std::to_string(props.weightCarrying) + "/" +
                  std::to_string(props.weightCapacity),
      }},
  });
  weightStyle.x = contentX + statsRowPadding;
  weightStyle.y = statsRowY + scaledStatsRowHeight / 2;
  weightText->build();
  modal->addChild(weightText);

  auto goldText = new TextLine(window, modal);
  goldText->setId("statsGold");
  auto& goldStyle = goldText->getStyle();
  setBaseFontConfig(goldStyle, BaseFontConfig::MODAL_TEXT);
  goldStyle.fontColor = Colors::DarkGrey;
  goldStyle.textAlign = TextAlign::LEFT_CENTER;
  goldStyle.scale = 1.f;
  goldText->setProps(TextLineProps{
      .textBlocks = {{
          .text = std::to_string(props.gold) + std::string(TRANSLATE(" gp")),
          .fontColor = Colors::Blue,
      }},
  });
  goldStyle.x = contentX + contentW - goldText->getDims().first - statsRowPadding;
  goldStyle.y = statsRowY + scaledStatsRowHeight / 2;
  goldText->build();
  modal->addChild(goldText);

  // Create SectionScrollable for content area
  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  BaseStyle scrollableStyle;
  scrollableStyle.x = contentX;
  scrollableStyle.y = scrollableY;
  scrollableStyle.width = unscaledContentW;
  scrollableStyle.height = scrollableHeight;
  scrollableStyle.scale = style.scale;
  scrollableSection->setStyle(scrollableStyle);
  scrollableSection->setProps(SectionScrollableProps{.scrollBarWidth = 40});
  addChild(scrollableSection);

  // Create ListInventory inside the scrollable section
  auto listInventory = new ListInventory(window, scrollableSection);
  listInventory->setId("listInventory");
  auto& listStyle = listInventory->getStyle();
  listStyle.x = 0; // Positioned relative to scrollable section
  listStyle.y = 0;
  listStyle.width = static_cast<float>(contentW) /
                        style.scale - // contentW and contentH are already scaled
                    scrollableSection->getProps().scrollBarWidth -
                    8;
  listStyle.scale = style.scale;

  ListInventoryProps listProps;
  listProps.characterPlayerId = props.characterPlayerId;
  populateInventoryProps(listProps.items);
  listInventory->setProps(listProps);
  scrollableSection->addChild(listInventory);
  scrollableSection->build();
}

void PageInventory::render(int dt) { UiElement::render(dt); }

} // namespace ui
