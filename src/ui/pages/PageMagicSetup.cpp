#include "PageMagicSetup.h"
#include "layers/ui/LayerMagic.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/PartyMemberIconSelector.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/helpers/modalLayoutFit.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include "ui/observers/ObserverShowLayerEquipRunes.hpp"
#include <algorithm>

namespace ui {

PageMagicSetup::PageMagicSetup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PageMagicSetup::setProps(const PageMagicSetupProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageMagicSetupProps& PageMagicSetup::getProps() { return props; }

const PageMagicSetupProps& PageMagicSetup::getProps() const { return props; }

const std::pair<int, int> PageMagicSetup::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageMagicSetup::addPortraitBackground(ModalStandard* modal) {
  if (props.characterPlayerSprite.empty()) {
    return;
  }
  auto* icon = modal->getChildById("headerIcon");
  if (!icon) {
    return;
  }

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
  const auto insertBefore =
      std::find_if(modalChildren.begin(),
                   modalChildren.end(),
                   [](const bmin::UniquePtr<UiElement>& child) {
                     return child->getId() == "headerIcon";
                   });
  if (insertBefore != modalChildren.end()) {
    modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
  }
}

void PageMagicSetup::addPartyMemberSelector(ModalStandard* modal) {
  if (props.partyMembers.empty()) {
    return;
  }

  auto [subtitleX, subtitleY] = modal->getSubTitleLocation();
  const int partyIconSize = ButtonClose::closeButtonSize;
  const int scaledPartyIconSize = static_cast<int>(partyIconSize * style.scale);
  const int partyIconY = subtitleY - scaledPartyIconSize / 2;

  PartyMemberIconSelectorProps selectorProps;
  selectorProps.selectedIndex = props.partyMemberMagicIndex;
  selectorProps.target = PartyMemberIconSelectorTarget::MAGIC;
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

bool PageMagicSetup::isNarrowLayout() const {
  if (props.width <= 0 || props.height <= 0) {
    return false;
  }
  return static_cast<float>(props.height) / static_cast<float>(props.width) >=
         narrowAspectMin;
}

int PageMagicSetup::equippedRowHeight() const {
  return std::max(manaSlotSize, editRunesButtonHeight) + statusStripPadding;
}

int PageMagicSetup::statusAreaHeight(bool narrow) const {
  if (narrow) {
    return equippedRowHeight() + statusStripPadding + availableRunesGridHeight;
  }
  return availableRunesGridHeight;
}

void PageMagicSetup::addRuneSlotRow(ModalStandard* modal,
                                   int contentX,
                                   int contentW,
                                   int rowY,
                                   int rowHeight,
                                   StatusAlign align) {
  const int scaledSlotSize = static_cast<int>(manaSlotSize * style.scale);
  const int scaledSlotGap = static_cast<int>(manaSlotGap * style.scale);
  const int scaledRowHeight = static_cast<int>(rowHeight * style.scale);
  const int scaledPadding = static_cast<int>(statusStripPadding * style.scale);
  const int scaledButtonW = static_cast<int>(editRunesButtonWidth * style.scale);
  const int scaledButtonH = static_cast<int>(editRunesButtonHeight * style.scale);
  const int scaledButtonGap = static_cast<int>(editRunesButtonGap * style.scale);
  const int slotY = rowY + (scaledRowHeight - scaledSlotSize) / 2;
  const int buttonY = rowY + (scaledRowHeight - scaledButtonH) / 2;
  const int scaledIconSize =
      static_cast<int>(manaSlotIconSize * manaSlotIconScale * style.scale);
  const int slotCount = static_cast<int>(props.runeSlots.size());
  const int slotsWidth =
      slotCount > 0 ? slotCount * scaledSlotSize + (slotCount - 1) * scaledSlotGap
                    : 0;
  const int rowContentWidth = scaledButtonW + scaledButtonGap + slotsWidth;
  int buttonX = contentX + scaledPadding;
  if (align == StatusAlign::Center) {
    buttonX = contentX + (contentW - rowContentWidth) / 2;
  } else if (align == StatusAlign::Right) {
    buttonX = contentX + contentW - scaledPadding - rowContentWidth;
  }
  const int stripX = buttonX + scaledButtonW + scaledButtonGap;

  auto editButton = new ButtonModal(window, modal);
  editButton->setId("editRunesButton");
  editButton->setPos(buttonX, buttonY);
  editButton->setScale(1.f);
  editButton->setProps(ButtonModalProps{
      .text = TRANSLATE("Runes"),
      .width = scaledButtonW,
      .height = scaledButtonH,
      .fontSize = sdl2w::TEXT_SIZE_14,
  });
  if (!props.characterPlayerId.empty()) {
    editButton->addEventObserver(
        new ObserverShowLayerEquipRunes(window, props.characterPlayerId));
  }
  modal->addChild(editButton);

  for (size_t i = 0; i < props.runeSlots.size(); ++i) {
    const auto& slot = props.runeSlots[i];
    const int slotX = stripX + static_cast<int>(i) * (scaledSlotSize + scaledSlotGap);
    const bool isSelected = props.selectedRuneSlotIndex == static_cast<int>(i);

    auto slotQuad = new Quad(window, modal);
    slotQuad->setId("runeSlot_" + bmin::toString(static_cast<int>(i)));
    slotQuad->setPos(slotX, slotY);
    slotQuad->setScale(1.f);
    slotQuad->setProps(QuadProps{
        .width = scaledSlotSize,
        .height = scaledSlotSize,
        .bgColor = slot.filled ? Colors::Grey2 : Colors::DarkGrey,
        .borderColor = isSelected ? Colors::ButtonModalSelected : Colors::Transparent,
        .borderSize = isSelected ? 2 : 0,
    });
    modal->addChild(slotQuad);

    if (slot.filled && !slot.iconSprite.empty()) {
      auto icon = new SpriteElement(window, slotQuad);
      icon->setId("runeSlotIcon");
      icon->setPos((scaledSlotSize - scaledIconSize) / 2,
                   (scaledSlotSize - scaledIconSize) / 2);
      icon->setScale(manaSlotIconScale * style.scale);
      icon->setProps(SpriteElementProps{
          .width = manaSlotIconSize,
          .height = manaSlotIconSize,
          .spriteName = slot.iconSprite,
      });
      slotQuad->addChild(icon);
    }
  }
}

void PageMagicSetup::addElementCountGrid(ModalStandard* modal,
                                         int contentX,
                                         int contentW,
                                         int gridTopY,
                                         StatusAlign align) {
  if (props.elementCounts.empty()) {
    return;
  }

  const int scaledCellW = static_cast<int>(elementCellWidth * style.scale);
  const int scaledCellH = static_cast<int>(elementCellHeight * style.scale);
  const int scaledPadding = static_cast<int>(statusStripPadding * style.scale);
  const int scaledIconSize =
      static_cast<int>(elementIconSize * elementIconScale * style.scale);
  const int gridWidth = elementGridCols * scaledCellW;
  int gridOriginX = contentX + scaledPadding;
  if (align == StatusAlign::Center) {
    gridOriginX = contentX + (contentW - gridWidth) / 2;
  } else if (align == StatusAlign::Right) {
    gridOriginX = contentX + contentW - scaledPadding - gridWidth;
  }

  TextFontProps countFont;
  setBaseFontConfig(countFont, BaseFontConfig::MODAL_TEXT);

  for (size_t i = 0; i < props.elementCounts.size(); ++i) {
    const auto& entry = props.elementCounts[i];
    const int col = static_cast<int>(i % elementGridCols);
    const int row = static_cast<int>(i / elementGridCols);
    const int cellX = gridOriginX + col * scaledCellW;
    const int cellY = gridTopY + row * scaledCellH;

    auto countText = new TextLine(window, modal);
    countText->setId("elementCount_" + bmin::toString(static_cast<int>(i)));
    countText->setScale(1.f);
    TextLineProps countProps;
    countProps.fontFamily = countFont.fontFamily;
    countProps.fontSize = sdl2w::TEXT_SIZE_14;
    countProps.fontColor = Colors::DarkGrey;
    countProps.textAlign = TextAlign::LEFT_TOP;
    countProps.textBlocks.pushBack({.text = bmin::toString(entry.count)});
    countText->setProps(countProps);
    auto [countW, countH] = countText->getDims();
    countText->setPos(cellX + (scaledCellW - countW) / 2, cellY);
    modal->addChild(countText);

    if (!entry.iconSprite.empty()) {
      auto icon = new SpriteElement(window, modal);
      icon->setId("elementIcon_" + bmin::toString(static_cast<int>(i)));
      icon->setPos(cellX + (scaledCellW - scaledIconSize) / 2, cellY + countH);
      icon->setScale(elementIconScale * style.scale);
      icon->setProps(SpriteElementProps{
          .width = elementIconSize,
          .height = elementIconSize,
          .spriteName = entry.iconSprite,
      });
      modal->addChild(icon);
    }
  }
}

ListMagicSpellsProps PageMagicSetup::makeSpellListProps(int width) const {
  ListMagicSpellsProps listProps;
  listProps.width = width;
  for (const auto& spell : props.spells) {
    bmin::String label = spell.label;
    label += " [";
    label += bmin::toString(spell.manaCost);
    label += "]";
    if (spell.ready) {
      label += " (r)";
    }
    listProps.spells.pushBack(ListMagicSpellsPropsSpell{
        .id = spell.id,
        .label = label,
        .iconSprite = spell.iconSprite,
        .requiredRuneSprites = spell.requiredRuneSprites,
    });
  }
  return listProps;
}

void PageMagicSetup::addSpellsPanel(int x, int y, int width, int height) {
  auto scrollable = new SectionScrollable(window, this);
  scrollable->setId("spellsSection");
  scrollable->setPos(x, y);
  scrollable->setScale(style.scale);
  scrollable->setProps(SectionScrollableProps{
      .width = width,
      .height = height,
      .scrollBarWidth = spellPanelScrollBarWidth,
      .bgColor = Colors::White,
  });
  addChild(scrollable);

  const int scaledPadding = static_cast<int>(spellPanelHeaderPadding * style.scale);
  const int listWidth = width - spellPanelScrollBarWidth - spellPanelHeaderPadding;

  auto header = new TextLine(window, scrollable);
  header->setId("spellsSection_header");
  header->setPos(scaledPadding, scaledPadding);
  header->setScale(1.f);
  TextFontProps headerFont;
  setBaseFontConfig(headerFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps headerProps;
  headerProps.fontFamily = headerFont.fontFamily;
  headerProps.fontSize = sdl2w::TEXT_SIZE_24;
  headerProps.fontColor = Colors::DarkBlue;
  headerProps.textAlign = TextAlign::LEFT_TOP;
  headerProps.textBlocks.pushBack({.text = TRANSLATE("Spells")});
  header->setProps(headerProps);
  scrollable->addChild(header);

  auto [headerW, headerH] = header->getDims();
  const int listY = scaledPadding + headerH;

  auto list = new ListMagicSpells(window, scrollable);
  list->setId("spellsSection_list");
  list->setPos(0, listY);
  list->setScale(style.scale);
  list->setProps(makeSpellListProps(listWidth));
  scrollable->addChild(list);

  scrollable->build();
}

void PageMagicSetup::build() {
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
  addChild(modal);

  addPortraitBackground(modal);

  auto closeButton = modal->getCloseButtonElement();
  if (closeButton) {
    closeButton->addEventObserver(
        new ObserverRemoveLayer(layers::LayerMagic::LAYER_ID));
  }

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  const int unscaledContentW = static_cast<int>(contentW / style.scale);
  const int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  if (props.characterPlayerLabel.empty()) {
    titleBlock.text = TRANSLATE("Magic");
  } else {
    titleBlock.text =
        bmin::String(TRANSLATE("Magic")) + " - " + props.characterPlayerLabel;
  }
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  addPartyMemberSelector(modal);

  // Portrait / narrow (e.g. 400x900): stack equipped runes above available runes.
  // Landscape: keep them side-by-side in one status strip.
  const bool narrow = isNarrowLayout();
  const int statusHeight = statusAreaHeight(narrow);
  const int scaledStatusHeight = static_cast<int>(statusHeight * style.scale);
  const int scrollableY = contentY + scaledStatusHeight;
  const int scrollableHeight = unscaledContentH - statusHeight;

  if (narrow) {
    const int equippedHeight = equippedRowHeight();
    const int scaledEquippedHeight =
        static_cast<int>(equippedHeight * style.scale);
    const int scaledGap = static_cast<int>(statusStripPadding * style.scale);
    const int gridTopY = contentY + scaledEquippedHeight + scaledGap;
    addRuneSlotRow(modal,
                   contentX,
                   contentW,
                   contentY,
                   equippedHeight,
                   StatusAlign::Center);
    addElementCountGrid(
        modal, contentX, contentW, gridTopY, StatusAlign::Center);
  } else {
    addRuneSlotRow(modal,
                   contentX,
                   contentW,
                   contentY,
                   availableRunesGridHeight,
                   StatusAlign::Left);
    addElementCountGrid(
        modal, contentX, contentW, contentY, StatusAlign::Right);
  }

  addSpellsPanel(contentX, scrollableY, unscaledContentW, scrollableHeight);
}

void PageMagicSetup::render(int dt) { UiElement::render(dt); }

} // namespace ui
