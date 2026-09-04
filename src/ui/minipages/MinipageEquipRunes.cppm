module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>

export module carcer.ui.minipages:MinipageEquipRunes;
export import bmin.containers;
import bmin.string_interop;
export import carcer.model.templates;
export import carcer.ui.core;
import sdl2w;
import carcer.actions;
import carcer.model.instances;
import carcer.ui.elements;
import carcer.ui.core;
import carcer.ui.helpers;
import carcer.ui.layouts;
import carcer.ui.pages.PageCharacter;
#include "macros.h"

export {

// --- from ui/minipages/MinipageEquipRunes.h ---
namespace ui {

struct MinipageEquipRunesSlot {
  bool filled = false;
  bmin::String iconSprite;
};

struct MinipageEquipRunesRow {
  model::RuneType type = model::RuneType::HEAT;
  bmin::String iconSprite;
  int availableCount = 0;
  int equippedCount = 0;
};

struct MinipageEquipRunesProps {
  int width = 0;
  int height = 0;
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::DynArray<MinipageEquipRunesSlot> runeSlots;
  bmin::DynArray<MinipageEquipRunesRow> runeRows;
};

/** ModalSmall editor: equipped strip + available rune +/- list; Okay commits, X cancels. */
class MinipageEquipRunes : public UiElement {
private:
  MinipageEquipRunesProps props;

  static constexpr int contentPadding = 8;
  static constexpr int nameRowHeight = 28;
  static constexpr int runeSlotSize = 24;
  static constexpr int runeSlotGap = 4;
  static constexpr int runeSlotIconSize = 24;
  static constexpr float runeSlotIconScale = 1.f;
  static constexpr int equippedRowHeight = 40;
  static constexpr int gridCols = 2;
  // Match PageCharacter +/- ButtonIcon size (32).
  static constexpr int adjustButtonSize = 32;
  static constexpr int cellHeight = adjustButtonSize;
  static constexpr int cellGapX = 8;
  static constexpr int cellGapY = 4;
  static constexpr int cellInnerGap = 2;
  static constexpr int countTextWidth = 20;
  static constexpr int footerButtonWidth = 100;
  static constexpr int modalBorderWidth = 2;
  static constexpr int modalHeaderHeight = 80;

  int runeCellContentWidth() const;
  int contentInnerWidth() const;
  int contentInnerHeight() const;
  int modalWidth() const;
  int modalHeight() const;

  void addEquippedSlots(UiElement* parent, int x, int y, int width);
  void addRuneGrid(UiElement* parent, int x, int y, int width);

public:
  MinipageEquipRunes(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageEquipRunes() override = default;

  void setProps(const MinipageEquipRunesProps& _props);
  MinipageEquipRunesProps& getProps();
  const MinipageEquipRunesProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverAdjustEquippedRune.hpp ---
namespace ui {

class ObserverAdjustEquippedRune : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  bmin::String characterPlayerId;
  model::RuneType runeType = model::RuneType::HEAT;
  int delta = 0;

public:
  ObserverAdjustEquippedRune(const bmin::String& _characterPlayerId,
                             model::RuneType _runeType,
                             int _delta)
      : characterPlayerId(_characterPlayerId), runeType(_runeType), delta(_delta) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverCancelEquipRunes.hpp ---
namespace ui {

class ObserverCancelEquipRunes : public ui::UiEventObserver,
                                 public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverCommitEquipRunes.hpp ---
namespace ui {

class ObserverCommitEquipRunes : public ui::UiEventObserver,
                                 public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

namespace ui {

MinipageEquipRunes::MinipageEquipRunes(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void MinipageEquipRunes::setProps(const MinipageEquipRunesProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipageEquipRunesProps& MinipageEquipRunes::getProps() { return props; }

const MinipageEquipRunesProps& MinipageEquipRunes::getProps() const { return props; }

const std::pair<int, int> MinipageEquipRunes::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

int MinipageEquipRunes::runeCellContentWidth() const {
  return adjustButtonSize + cellInnerGap + runeSlotIconSize + cellInnerGap +
         countTextWidth + cellInnerGap + adjustButtonSize;
}

int MinipageEquipRunes::contentInnerWidth() const {
  const int slotCount = std::max(1, static_cast<int>(props.runeSlots.size()));
  const int equippedW =
      slotCount * runeSlotSize + std::max(0, slotCount - 1) * runeSlotGap;
  const int gridW =
      gridCols * runeCellContentWidth() + (gridCols - 1) * cellGapX;
  const int footerW = footerButtonWidth + 8 + 4;
  return std::max({equippedW, gridW, footerW}) + 2 * contentPadding;
}

int MinipageEquipRunes::contentInnerHeight() const {
  const int rowCount =
      props.runeRows.empty()
          ? 0
          : (static_cast<int>(props.runeRows.size()) + gridCols - 1) / gridCols;
  const int gridH =
      rowCount > 0 ? rowCount * cellHeight + (rowCount - 1) * cellGapY : 0;
  return nameRowHeight + equippedRowHeight + contentPadding + gridH +
         contentPadding;
}

int MinipageEquipRunes::modalWidth() const {
  return contentInnerWidth() + 2 * modalBorderWidth;
}

int MinipageEquipRunes::modalHeight() const {
  // ModalSmall content area = modalH - header - 2*border - BUTTONS_AREA_HEIGHT.
  return contentInnerHeight() + ModalSmall::BUTTONS_AREA_HEIGHT +
         modalHeaderHeight + 2 * modalBorderWidth;
}

void MinipageEquipRunes::addEquippedSlots(UiElement* parent, int x, int y, int width) {
  const int scaledSlotSize = static_cast<int>(runeSlotSize * style.scale);
  const int scaledSlotGap = static_cast<int>(runeSlotGap * style.scale);
  const int scaledIconSize =
      static_cast<int>(runeSlotIconSize * runeSlotIconScale * style.scale);
  const int slotCount = static_cast<int>(props.runeSlots.size());
  const int slotsWidth =
      slotCount > 0 ? slotCount * scaledSlotSize + (slotCount - 1) * scaledSlotGap
                    : 0;
  const int startX = x + (width - slotsWidth) / 2;

  for (size_t i = 0; i < props.runeSlots.size(); ++i) {
    const auto& slot = props.runeSlots[i];
    const int slotX = startX + static_cast<int>(i) * (scaledSlotSize + scaledSlotGap);

    auto slotQuad = new Quad(window, parent);
    slotQuad->setId("runeSlot_" + bmin::toString(static_cast<int>(i)));
    slotQuad->setPos(slotX, y);
    slotQuad->setScale(1.f);
    slotQuad->setProps(QuadProps{
        .width = scaledSlotSize,
        .height = scaledSlotSize,
        .bgColor = slot.filled ? Colors::Grey2 : Colors::DarkGrey,
    });
    parent->addChild(slotQuad);

    if (slot.filled && !slot.iconSprite.empty()) {
      auto icon = new SpriteElement(window, slotQuad);
      icon->setId("runeSlotIcon");
      icon->setPos((scaledSlotSize - scaledIconSize) / 2,
                   (scaledSlotSize - scaledIconSize) / 2);
      icon->setScale(runeSlotIconScale * style.scale);
      icon->setProps(SpriteElementProps{
          .width = runeSlotIconSize,
          .height = runeSlotIconSize,
          .spriteName = slot.iconSprite,
      });
      slotQuad->addChild(icon);
    }
  }
}

void MinipageEquipRunes::addRuneGrid(UiElement* parent, int x, int y, int width) {
  // Same ButtonIcon +/- sprites/sizing as PageCharacter stat modifiers.
  const int scaledBtn = static_cast<int>(adjustButtonSize * style.scale);
  const int scaledIcon =
      static_cast<int>(runeSlotIconSize * runeSlotIconScale * style.scale);
  const int scaledInnerGap = static_cast<int>(cellInnerGap * style.scale);
  const int scaledCountW = static_cast<int>(countTextWidth * style.scale);
  const int scaledCellH = static_cast<int>(cellHeight * style.scale);
  const int scaledGapX = static_cast<int>(cellGapX * style.scale);
  const int scaledGapY = static_cast<int>(cellGapY * style.scale);

  // Compact cell: [-][icon][count][+] with tight gaps; all vertically centered.
  const int cellContentW =
      scaledBtn + scaledInnerGap + scaledIcon + scaledInnerGap + scaledCountW +
      scaledInnerGap + scaledBtn;
  const int cellW = (width - scaledGapX) / gridCols;
  const int gridWidth = gridCols * cellW + (gridCols - 1) * scaledGapX;
  const int gridOriginX = x + (width - gridWidth) / 2;

  TextFontProps countFont;
  setBaseFontConfig(countFont, BaseFontConfig::MODAL_TEXT);

  const int equippedTotal = [&]() {
    int total = 0;
    for (const auto& slot : props.runeSlots) {
      if (slot.filled) {
        ++total;
      }
    }
    return total;
  }();

  for (size_t i = 0; i < props.runeRows.size(); ++i) {
    const auto& row = props.runeRows[i];
    const int col = static_cast<int>(i % gridCols);
    const int gridRow = static_cast<int>(i / gridCols);
    const int cellX = gridOriginX + col * (cellW + scaledGapX);
    const int cellY = y + gridRow * (scaledCellH + scaledGapY);
    const int contentX = cellX + (cellW - cellContentW) / 2;
    const int btnY = cellY + (scaledCellH - scaledBtn) / 2;
    const int iconY = cellY + (scaledCellH - scaledIcon) / 2;
    const int remainingCount =
        std::max(0, row.availableCount - row.equippedCount);

    const bool canMinus = row.equippedCount > 0;
    const bool canPlus =
        equippedTotal < static_cast<int>(model::CharacterPlayer::kRuneSlotCount) &&
        remainingCount > 0;

    auto minusBtn = new ButtonIcon(window, parent);
    minusBtn->setId("minus_" + bmin::toString(static_cast<int>(i)));
    minusBtn->setPos(contentX, btnY);
    minusBtn->setScale(style.scale);
    minusBtn->setProps(ButtonIconProps{
        .regularSprite = ButtonIcon::MINUS_ICON1,
        .activeSprite = ButtonIcon::MINUS_ICON2,
        .iconSize = adjustButtonSize,
        .isDisabled = !canMinus,
    });
    if (canMinus && !props.characterPlayerId.empty()) {
      minusBtn->addEventObserver(new ObserverAdjustEquippedRune(
          props.characterPlayerId, row.type, -1));
    }
    parent->addChild(minusBtn);

    int cursorX = contentX + scaledBtn + scaledInnerGap;
    if (!row.iconSprite.empty()) {
      auto icon = new SpriteElement(window, parent);
      icon->setId("icon_" + bmin::toString(static_cast<int>(i)));
      icon->setPos(cursorX, iconY);
      icon->setScale(runeSlotIconScale * style.scale);
      icon->setProps(SpriteElementProps{
          .width = runeSlotIconSize,
          .height = runeSlotIconSize,
          .spriteName = row.iconSprite,
      });
      parent->addChild(icon);
    }
    cursorX += scaledIcon + scaledInnerGap;

    auto countText = new TextLine(window, parent);
    countText->setId("count_" + bmin::toString(static_cast<int>(i)));
    countText->setScale(1.f);
    TextLineProps countProps;
    countProps.fontFamily = countFont.fontFamily;
    countProps.fontSize = sdl2w::TEXT_SIZE_18;
    countProps.fontColor = Colors::DarkGrey;
    countProps.textAlign = TextAlign::CENTER;
    countProps.textBlocks.pushBack({.text = bmin::toString(remainingCount)});
    countText->setProps(countProps);
    countText->setPos(cursorX + scaledCountW / 2, cellY + scaledCellH / 2);
    parent->addChild(countText);
    cursorX += scaledCountW + scaledInnerGap;

    auto plusBtn = new ButtonIcon(window, parent);
    plusBtn->setId("plus_" + bmin::toString(static_cast<int>(i)));
    plusBtn->setPos(cursorX, btnY);
    plusBtn->setScale(style.scale);
    plusBtn->setProps(ButtonIconProps{
        .regularSprite = ButtonIcon::PLUS_ICON1,
        .activeSprite = ButtonIcon::PLUS_ICON2,
        .iconSize = adjustButtonSize,
        .isDisabled = !canPlus,
    });
    if (canPlus && !props.characterPlayerId.empty()) {
      plusBtn->addEventObserver(new ObserverAdjustEquippedRune(
          props.characterPlayerId, row.type, +1));
    }
    parent->addChild(plusBtn);
  }
}

void MinipageEquipRunes::build() {
  children.clear();

  // props.width/height are window dims from the layer; size modal to content.
  const int windowW = props.width > 0 ? props.width : style.width;
  const int windowH = props.height > 0 ? props.height : style.height;
  const int fittedW = modalWidth();
  const int fittedH = modalHeight();
  const int modalX = std::max(0, (windowW - fittedW) / 2);
  const int modalY = std::max(0, (windowH - fittedH) / 2);

  style.x = modalX;
  style.y = modalY;
  style.width = fittedW;
  style.height = fittedH;

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  modal->setPos(modalX, modalY);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = fittedW,
      .height = fittedH,
      .layoutFit = LayoutFit::FullBleed,
  });
  addChild(modal);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  titleProps.textBlocks.pushBack({.text = TRANSLATE("Equip Runes")});
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto closeButton = modal->getCloseButtonElement();
  if (closeButton) {
    closeButton->addEventObserver(new ObserverCancelEquipRunes());
  }

  const int contentW = modal->getContentDims().first;
  auto [contentX, contentY] = modal->getContentLocation();
  const int scaledPad = static_cast<int>(contentPadding * style.scale);
  const int scaledNameH = static_cast<int>(nameRowHeight * style.scale);
  const int scaledEquippedH = static_cast<int>(equippedRowHeight * style.scale);

  auto nameText = new TextLine(window, modal);
  nameText->setId("characterName");
  nameText->setScale(1.f);
  TextFontProps nameFont;
  setBaseFontConfig(nameFont, BaseFontConfig::MODAL_TEXT);
  TextLineProps nameProps;
  nameProps.fontFamily = nameFont.fontFamily;
  nameProps.fontSize = sdl2w::TEXT_SIZE_18;
  nameProps.fontColor = Colors::DarkGrey;
  nameProps.textAlign = TextAlign::CENTER;
  bmin::String nameLabel = props.characterPlayerLabel;
  if (nameLabel.empty()) {
    nameLabel = TRANSLATE("Unknown");
  }
  nameProps.textBlocks.pushBack({.text = nameLabel});
  nameText->setProps(nameProps);
  nameText->setPos(contentX + contentW / 2, contentY + scaledNameH / 2);
  modal->addChild(nameText);

  const int equippedY = contentY + scaledNameH;
  addEquippedSlots(modal,
                   contentX + scaledPad,
                   equippedY + (scaledEquippedH -
                                static_cast<int>(runeSlotSize * style.scale)) /
                                   2,
                   contentW - 2 * scaledPad);

  const int gridY = equippedY + scaledEquippedH + scaledPad;
  addRuneGrid(modal, contentX + scaledPad, gridY, contentW - 2 * scaledPad);

  auto [buttonsW, buttonsH] = modal->getButtonsDims();
  auto [buttonsX, buttonsY] = modal->getButtonsLocation();
  const int buttonPadding = 2;
  const int buttonHeight = ModalSmall::BUTTONS_AREA_HEIGHT;

  auto buttonGroup = new ButtonGroup(window, modal);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(buttonsX, buttonsY);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(ButtonGroupProps{
      .width = static_cast<int>(buttonsW / style.scale),
      .alignment = ButtonGroupAlignment::RIGHT,
      .buttonWidth = footerButtonWidth,
      .buttonHeight = buttonHeight - 2 * buttonPadding,
      .padding = buttonPadding,
      .buttons =
          {
              {.label = TRANSLATE("Okay"), .type = ButtonGroupButtonType::MODAL},
          },
  });
  buttonGroup->addObserverToButtonAtIndex(0, new ObserverCommitEquipRunes());
  modal->addChild(buttonGroup);
}

void MinipageEquipRunes::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverAdjustEquippedRune::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto* stateManager = getStateManager();
    if (!stateManager || characterPlayerId.empty() || delta == 0) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiAdjustEquippedRune(characterPlayerId, runeType, delta),
        0);
  }

void ObserverCancelEquipRunes::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiCancelEquipRunes(), 0);
  }

void ObserverCommitEquipRunes::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiCommitEquipRunes(), 0);
  }

} // namespace ui
