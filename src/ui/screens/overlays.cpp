module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <vector>

module carcer.ui.screens;
#include "macros.h"

namespace ui {

PopupDropConfirm::PopupDropConfirm(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

PopupDropConfirm::~PopupDropConfirm() = default;

void PopupDropConfirm::setProps(const PopupDropConfirmProps& _props) {
  props = _props;
  //   confirmObserver =
  //       bmin::makeUnique<ObserverDropInventoryItem>(props.characterPlayerId,
  //       props.itemId);
  //   cancelObserver =
  //       bmin::makeUnique<ObserverRemoveLayer>(state::LayerId::DropConfirm);
  build();
}

PopupDropConfirmProps& PopupDropConfirm::getProps() { return props; }

const PopupDropConfirmProps& PopupDropConfirm::getProps() const { return props; }

void PopupDropConfirm::build() {
  children.clear();

  auto modal = new ConfirmModal(window, this);
  modal->setId("confirmModal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);

  const bmin::String messageText =
      TRANSLATE("Are you sure you wish to drop ") + props.itemLabel + "?";
  modal->setProps(ConfirmModalProps{
      .title = TRANSLATE("Drop"),
      .message = messageText,
  });
  modal->getButtonGroup()->addObserverToButtonAtIndex(
      1, new ObserverDropInventoryItem(props.characterPlayerId, props.itemId));
  modal->getButtonGroup()->addObserverToButtonAtIndex(
      0, new ObserverRemoveLayer(state::LayerId::DropConfirm));

  auto [modalW, modalH] = modal->getDims();
  style.width = modalW / style.scale;
  style.height = modalH / style.scale;

  addChild(modal);
}

void PopupDropConfirm::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverDropInventoryItem::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverDropInventoryItem::onClick item=" << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::dropInventoryItem(characterPlayerId, itemId),
        0);
  }

} // namespace ui


namespace ui {

PopupGive::PopupGive(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void PopupGive::setProps(const PopupGiveProps& _props) {
  props = _props;
  build();
}

PopupGiveProps& PopupGive::getProps() { return props; }

const PopupGiveProps& PopupGive::getProps() const { return props; }

int PopupGive::getSelectedQuantity() const {
  if (quantitySlider) {
    return quantitySlider->getProps().value;
  }
  return props.selectedQuantity;
}

void PopupGive::build() {
  children.clear();
  quantitySlider = nullptr;

  style.width = 320;
  const int padding = 8;
  const int closeButtonSize = 32;
  const int sliderAreaHeight = 64;
  const int scaledWidth = style.width * style.scale;
  const int paddingScaled = padding * style.scale;
  const int contentWidth = style.width - 2 * padding;

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  closeButton->setPos(
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale,
      style.y + padding * style.scale);
  closeButton->setScale(style.scale);
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(
      new ObserverRemoveLayer(state::LayerId::GiveContext));
  addChild(closeButton);

  auto title = new TextLine(window, this);
  title->setId("title");
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  title->setPos(style.x + padding * style.scale,
                style.y + padding * style.scale + (closeButtonSize * style.scale) / 2);
  title->setScale(1.f);
  bmin::String titleText = TRANSLATE("Give");
  if (!props.itemLabel.empty()) {
    titleText += " " + props.itemLabel;
  }
  title->setProps(TextLineProps{
      .textBlocks =
          {
              {
                  .text = titleText,
              },
          },
      .fontFamily = titleFont.fontFamily,
      .fontSize = titleFont.fontSize,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  addChild(title);
  auto [titleWidth, titleHeight] = title->calculateTextDims();

  int contentY = style.y + paddingScaled + titleHeight + paddingScaled * 2;

  if (props.showQuantitySlider) {
    auto slider = new HorizontalSlider(window, this);
    slider->setId("quantitySlider");
    quantitySlider = slider;
    slider->setPos(style.x + padding * style.scale, contentY);
    slider->setScale(style.scale);
    slider->setProps(HorizontalSliderProps{
        .minValue = 1,
        .maxValue = std::max(1, props.maxQuantity),
        .value = std::clamp(props.selectedQuantity, 1, std::max(1, props.maxQuantity)),
        .width = style.width - 2 * padding,
        .height = sliderAreaHeight,
        .sliderBarHeight = 32,
        .indicatorWidth = 24,
        .labelColor = Colors::Black,
    });
    addChild(slider);
    contentY += sliderAreaHeight * style.scale + paddingScaled;
  }

  auto list = new VerticalList(window, this);
  list->setId("partyList");
  list->setPos(style.x + padding * style.scale, contentY);
  list->setScale(style.scale);

  const int iconSize = 32;
  const int rowHeight = 32;
  const int rowGap = 2;
  const int rowWidth = style.width - 2 * padding;
  const int buttonWidth = rowWidth - iconSize;
  for (const auto& member : props.partyMembers) {
    auto row = new Quad(window, list);
    row->setId("row-" + member.characterPlayerId);
    row->setScale(style.scale);
    row->setProps(QuadProps{
        .width = rowWidth,
        .height = rowHeight,
        .bgColor = Colors::Transparent,
    });

    auto button = new ButtonModal(window, row);
    button->setId("btn-" + member.characterPlayerId);
    button->setPos(iconSize, 0);
    button->setScale(style.scale);
    button->setProps(ButtonModalProps{
        .text = member.label,
        .width = buttonWidth,
        .height = rowHeight,
    });
    button->addEventObserver(new ObserverGiveInventoryItem(
        member.characterPlayerId, props.fromCharacterPlayerId, props.itemId, this));
    row->addChild(button);

    auto iconBg = new OutsetRectangle(window, row);
    iconBg->setId("iconBg-" + member.characterPlayerId);
    iconBg->setPos(0, 0);
    iconBg->setScale(style.scale);
    iconBg->setProps(OutsetRectangleProps{
        .width = iconSize,
        .height = iconSize,
        .color = Colors::LightGrey,
        .colorTopRight = Colors::White,
        .colorBottomLeft = Colors::ButtonModalGrey2,
        .borderSize = 0,
    });
    row->addChild(iconBg);

    auto sprite = new Quad(window, iconBg);
    sprite->setId("sprite-" + member.characterPlayerId);
    sprite->setPos(0, 0);
    sprite->setScale(style.scale);
    sprite->setProps(QuadProps{
        .width = iconSize,
        .height = iconSize,
        .bgSprite = member.spriteName,
    });
    iconBg->addChild(sprite);

    list->addListItem(row);
  }

  list->setProps({
      .width = contentWidth,
      .lineHeight = static_cast<int>((rowHeight + rowGap) * style.scale),
      .lineGap = rowGap,
      .bgColor = Colors::Transparent,
  });
  auto [listW, listH] = list->getDims();
  addChild(list);

  style.height = ((contentY + listH + paddingScaled) - style.y) / style.scale;

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  children.insert(children.begin(), bmin::UniquePtr<UiElement>(border));
}

void PopupGive::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverGiveInventoryItem::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverGiveInventoryItem::onClick to=" << toCharacterPlayerId
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || popupGive == nullptr) {
      return;
    }
    const int quantity = popupGive->getSelectedQuantity();
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::giveInventoryItem(
            fromCharacterPlayerId, toCharacterPlayerId, itemId, quantity),
        0);
  }

} // namespace ui


namespace ui {

PopupInventoryItem::PopupInventoryItem(sdl2w::Window* _window,
                                       UiElement* _parent,
                                       PopupOrientation _orientation)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  props.orientation = _orientation;
  build();
}

void PopupInventoryItem::setProps(const PopupInventoryItemProps& _props) {
  props = _props;
  build();
}

PopupInventoryItemProps& PopupInventoryItem::getProps() { return props; }

const PopupInventoryItemProps& PopupInventoryItem::getProps() const { return props; }

void PopupInventoryItem::build() {
  children.clear();

  if (props.orientation == PopupOrientation::NARROW) {
    style.width = 300;
    style.height = 350;
  } else {
    style.width = 400;
    style.height = 250;
  }

  const int padding = 4;
  const int closeButtonSize = 32;
  const int actionButtonsWidth = 80;
  const int actionButtonsHeight = 32;
  const int iconBgSize = 32;
  const int itemIconSize = 16;
  const int vertSpacerHeight = 12;
  const int buttonVertSpacerHeight = 2;

  auto [scaledWidth, scaledHeight] = getDims();

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  addChild(border);

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  closeButton->setPos(
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale,
      style.y + padding * style.scale);
  closeButton->setScale(style.scale);
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(
      new ObserverRemoveLayer(state::LayerId::InventoryContext));
  addChild(closeButton);

  int actionButtonHeightTotalScaled =
      actionButtonsHeight * style.scale + buttonVertSpacerHeight * style.scale;
  const int buttonsX =
      scaledWidth - padding * style.scale - actionButtonsWidth * style.scale;
  int buttonsY = padding * style.scale + closeButtonSize * style.scale +
                 3 * actionButtonHeightTotalScaled;
  if (props.orientation == PopupOrientation::NARROW) {
    buttonsY += 175 * style.scale;
  } else {
    buttonsY += 75 * style.scale;
  }

  auto createButton = [&](const bmin::String& text, int _x, int _y) {
    auto button = new ButtonModal(window, this);
    button->setId(text);
    button->setPos(style.x + _x, style.y + _y);
    button->setScale(style.scale);
    button->setProps(ButtonModalProps{
        .text = text,
        .width = actionButtonsWidth,
        .height = actionButtonsHeight,
    });
    return button;
  };

  auto giveButton = createButton(TRANSLATE("Give"), buttonsX, buttonsY);
  giveButton->addEventObserver(new ObserverShowLayerGiveContext(
      window, props.characterPlayerId, props.item.id));
  addChild(giveButton);
  buttonsY -= actionButtonHeightTotalScaled;

  auto dropButton = createButton(TRANSLATE("Drop"), buttonsX, buttonsY);
  dropButton->addEventObserver(new ObserverShowLayerDropContext(
      window, props.characterPlayerId, props.item.id));
  addChild(dropButton);
  buttonsY -= actionButtonHeightTotalScaled;

  if (props.equippable) {
    auto equipButton = createButton(TRANSLATE("Equip"), buttonsX, buttonsY);
    addChild(equipButton);
    buttonsY -= actionButtonHeightTotalScaled;
  }

  if (props.usable) {
    auto useButton = createButton(TRANSLATE("Use"), buttonsX, buttonsY);
    addChild(useButton);
    buttonsY -= actionButtonHeightTotalScaled;
  }

  auto spriteBgQuad = new Quad(window, this);
  spriteBgQuad->setId("spriteBg");
  spriteBgQuad->setPos(style.x + padding * style.scale, style.y + padding * style.scale);
  spriteBgQuad->setScale(style.scale);
  spriteBgQuad->setProps(QuadProps{
      .width = iconBgSize,
      .height = iconBgSize,
      .bgColor = Colors::LightGrey,
  });
  addChild(spriteBgQuad);

  auto spriteQuad = new Quad(window, this);
  spriteQuad->setId("icon");
  spriteQuad->setPos(iconBgSize / 2.f - (itemIconSize / 2.f) * 2.f * style.scale,
                     iconBgSize / 2.f - (itemIconSize / 2.f) * 2.f * style.scale);
  spriteQuad->setScale(2.f * style.scale);
  spriteQuad->setProps(QuadProps{
      .width = itemIconSize,
      .height = itemIconSize,
      .bgSprite = props.spriteName,
  });
  spriteBgQuad->addChild(spriteQuad);

  auto label = new TextLine(window, this);
  label->setId("label");
  TextFontProps labelFont;
  setBaseFontConfig(labelFont, BaseFontConfig::MODAL_TITLE);
  label->setPos(
      style.x + padding * style.scale + iconBgSize * style.scale + padding * style.scale,
      style.y + padding * style.scale + (iconBgSize * style.scale) / 2);
  label->setScale(1.f);
  label->setProps(TextLineProps{
      .textBlocks =
          {
              {
                  .text = props.label,
              },
          },
      .fontFamily = labelFont.fontFamily,
      .fontSize = labelFont.fontSize,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  addChild(label);

  auto itemInfo = new ItemInfo(window, this);
  itemInfo->setId("itemInfo");
  itemInfo->setPos(style.x + padding * style.scale,
                   style.y + padding * style.scale + iconBgSize * style.scale +
                       vertSpacerHeight * style.scale);
  itemInfo->setScale(1.f);
  itemInfo->setProps(ItemInfoProps{
      .width = static_cast<int>(
          (scaledWidth - padding * 2 * style.scale - actionButtonsWidth * style.scale -
           8 * style.scale) /
          style.scale),
      .description = props.description,
      .weight = props.weight,
      .value = props.value,
  });
  addChild(itemInfo);
}

void PopupInventoryItem::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverShowLayerDropContext::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverShowLayerDropContext::onClick " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::showLayerDropContext(window, characterPlayerId, itemId),
        0);
  }

void ObserverShowLayerGiveContext::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverShowLayerGiveContext::onClick " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::showLayerGiveContext(
            window, fromCharacterPlayerId, itemId),
        0);
  }

} // namespace ui


namespace ui {

class PopupPickupItemCloseButtonObserver : public UiEventObserver,
                                           public state::StateManagerInterface {

  bmin::String closeLayerId;

public:
  explicit PopupPickupItemCloseButtonObserver(bmin::String _closeLayerId)
      : closeLayerId(std::move(_closeLayerId)) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "PopupPickupItemCloseButtonObserver::onClick" << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || closeLayerId.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::removeLayer(closeLayerId), 0);
  }
};

PopupPickupItem::PopupPickupItem(sdl2w::Window* _window,
                                 bmin::String _closeLayerId,
                                 PopupOrientation _orientation)
    : UiElement(_window, nullptr), closeLayerId(std::move(_closeLayerId)) {
  shouldPropagateEventsToChildren = true;
  props.orientation = _orientation;
  build();
}

void PopupPickupItem::setProps(const PopupPickupItemProps& _props) {
  props = _props;
  build();
}

PopupPickupItemProps& PopupPickupItem::getProps() { return props; }

const PopupPickupItemProps& PopupPickupItem::getProps() const { return props; }

void PopupPickupItem::build() {
  children.clear();

  if (props.orientation == PopupOrientation::NARROW) {
    style.width = 300;
    style.height = 250;
  } else {
    style.width = 300;
    style.height = 250;
  }

  const int padding = 4;
  const int closeButtonSize = 32;
  const int iconBgSize = 32;
  const int itemIconSize = 16;
  const int vertSpacerHeight = 12;

  auto [scaledWidth, scaledHeight] = getDims();

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
      .backgroundColor = Colors::White,
      .shadowColor = Colors::Black,
      .shadowOffsetX = -8,
      .shadowOffsetY = 8,
      .borderSize = 2,
  });
  addChild(border);

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  closeButton->setPos(
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale,
      style.y + padding * style.scale);
  closeButton->setScale(style.scale);
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(new PopupPickupItemCloseButtonObserver(closeLayerId));
  addChild(closeButton);

  auto spriteBgQuad = new Quad(window, this);
  spriteBgQuad->setId("spriteBg");
  spriteBgQuad->setPos(style.x + padding * style.scale, style.y + padding * style.scale);
  spriteBgQuad->setScale(style.scale);
  spriteBgQuad->setProps(QuadProps{
      .width = iconBgSize,
      .height = iconBgSize,
      .bgColor = Colors::LightGrey,
  });
  addChild(spriteBgQuad);

  auto spriteQuad = new Quad(window, this);
  spriteQuad->setId("icon");
  spriteQuad->setPos(iconBgSize / 2.f - (itemIconSize / 2.f) * 2.f * style.scale,
                     iconBgSize / 2.f - (itemIconSize / 2.f) * 2.f * style.scale);
  spriteQuad->setScale(2.f * style.scale);
  spriteQuad->setProps(QuadProps{
      .width = itemIconSize,
      .height = itemIconSize,
      .bgSprite = props.spriteName,
  });
  spriteBgQuad->addChild(spriteQuad);

  auto label = new TextLine(window, this);
  label->setId("label");
  TextFontProps labelFont;
  setBaseFontConfig(labelFont, BaseFontConfig::MODAL_TITLE);
  label->setPos(
      style.x + padding * style.scale + iconBgSize * style.scale + padding * style.scale,
      style.y + padding * style.scale + (iconBgSize * style.scale) / 2);
  label->setScale(1.f);
  label->setProps(TextLineProps{
      .textBlocks =
          {
              {
                  .text = props.label,
              },
          },
      .fontFamily = labelFont.fontFamily,
      .fontSize = labelFont.fontSize,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  addChild(label);

  auto itemInfo = new ItemInfo(window, this);
  itemInfo->setId("itemInfo");
  itemInfo->setPos(style.x + padding * style.scale,
                   style.y + padding * style.scale + iconBgSize * style.scale +
                       vertSpacerHeight * style.scale);
  itemInfo->setScale(style.scale);
  itemInfo->setProps(ItemInfoProps{
      .width = static_cast<int>((scaledWidth - padding * 2 * style.scale - 8 * style.scale) /
                                style.scale),
      .description = props.description,
      .weight = props.weight,
      .value = props.value,
  });
  addChild(itemInfo);
}

void PopupPickupItem::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

PopupSpellInfo::PopupSpellInfo(sdl2w::Window* _window,
                               UiElement* _parent,
                               PopupOrientation _orientation)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  props.orientation = _orientation;
  build();
}

void PopupSpellInfo::setProps(const PopupSpellInfoProps& _props) {
  props = _props;
  build();
}

PopupSpellInfoProps& PopupSpellInfo::getProps() { return props; }

const PopupSpellInfoProps& PopupSpellInfo::getProps() const { return props; }

void PopupSpellInfo::build() {
  children.clear();

  if (props.orientation == PopupOrientation::NARROW) {
    style.width = 300;
    style.height = 320;
  } else {
    style.width = 400;
    style.height = 260;
  }

  const int padding = 8;
  const int closeButtonSize = 32;
  const int iconBgSize = 32;
  const int spellIconSize = 24;
  const int runeIconSize = 24;
  const int vertSpacer = 8;

  auto [scaledWidth, scaledHeight] = getDims();

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  addChild(border);

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  closeButton->setPos(
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale,
      style.y + padding * style.scale);
  closeButton->setScale(style.scale);
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(
      new ObserverRemoveLayer(state::LayerId::SpellInfo));
  addChild(closeButton);

  auto spriteBgQuad = new Quad(window, this);
  spriteBgQuad->setId("spriteBg");
  spriteBgQuad->setPos(style.x + padding * style.scale, style.y + padding * style.scale);
  spriteBgQuad->setScale(style.scale);
  spriteBgQuad->setProps(QuadProps{
      .width = iconBgSize,
      .height = iconBgSize,
      .bgColor = Colors::LightGrey,
  });
  addChild(spriteBgQuad);

  if (!props.spriteName.empty()) {
    const int iconOffset = (iconBgSize - spellIconSize) / 2;
    auto sprite = new SpriteElement(window, spriteBgQuad);
    sprite->setId("icon");
    sprite->setPos(iconOffset, iconOffset);
    sprite->setScale(style.scale);
    sprite->setProps(SpriteElementProps{
        .width = spellIconSize,
        .height = spellIconSize,
        .spriteName = props.spriteName,
    });
    spriteBgQuad->addChild(sprite);
  }

  auto label = new TextLine(window, this);
  label->setId("label");
  TextFontProps labelFont;
  setBaseFontConfig(labelFont, BaseFontConfig::MODAL_TITLE);
  label->setPos(
      style.x + padding * style.scale + iconBgSize * style.scale + padding * style.scale,
      style.y + padding * style.scale + (iconBgSize * style.scale) / 2);
  label->setScale(1.f);
  label->setProps(TextLineProps{
      .textBlocks = {{.text = props.label}},
      .fontFamily = labelFont.fontFamily,
      .fontSize = labelFont.fontSize,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  addChild(label);

  TextFontProps bodyFont;
  setBaseFontConfig(bodyFont, BaseFontConfig::MODAL_TEXT);

  int contentY = style.y + padding * style.scale + iconBgSize * style.scale +
                 vertSpacer * style.scale;
  const int contentX = style.x + padding * style.scale;
  const int contentWidth =
      static_cast<int>((scaledWidth - padding * 2 * style.scale) / style.scale);

  auto manaLine = new TextLine(window, this);
  manaLine->setId("manaCost");
  manaLine->setPos(contentX, contentY);
  manaLine->setScale(1.f);
  manaLine->setProps(TextLineProps{
      .textBlocks =
          {{.text = TRANSLATE("Mana: ") + bmin::toString(props.manaCost)}},
      .fontFamily = bodyFont.fontFamily,
      .fontSize = bodyFont.fontSize,
      .fontColor = Colors::DarkBlue,
      .textAlign = TextAlign::LEFT_TOP,
  });
  addChild(manaLine);
  contentY += manaLine->getDims().second + vertSpacer * style.scale;

  auto runesHeader = new TextLine(window, this);
  runesHeader->setId("runesHeader");
  runesHeader->setPos(contentX, contentY);
  runesHeader->setScale(1.f);
  runesHeader->setProps(TextLineProps{
      .textBlocks = {{.text = TRANSLATE("Required runes:")}},
      .fontFamily = bodyFont.fontFamily,
      .fontSize = bodyFont.fontSize,
      .fontColor = Colors::DarkGrey,
      .textAlign = TextAlign::LEFT_TOP,
  });
  addChild(runesHeader);
  contentY += runesHeader->getDims().second + 4 * style.scale;

  if (props.requiredRunes.empty()) {
    auto noneLine = new TextLine(window, this);
    noneLine->setId("runesNone");
    noneLine->setPos(contentX, contentY);
    noneLine->setScale(1.f);
    noneLine->setProps(TextLineProps{
        .textBlocks = {{.text = TRANSLATE("None")}},
        .fontFamily = bodyFont.fontFamily,
        .fontSize = bodyFont.fontSize,
        .fontColor = Colors::DarkGrey,
        .textAlign = TextAlign::LEFT_TOP,
    });
    addChild(noneLine);
    contentY += noneLine->getDims().second + vertSpacer * style.scale;
  } else {
    int runeX = contentX;
    const int runeGap = static_cast<int>(4 * style.scale);
    const int groupGap = static_cast<int>(10 * style.scale);
    for (size_t i = 0; i < props.requiredRunes.size(); ++i) {
      const auto& req = props.requiredRunes[i];
      const int count = req.count > 0 ? req.count : 1;
      for (int n = 0; n < count; ++n) {
        if (!req.iconSprite.empty()) {
          auto runeIcon = new SpriteElement(window, this);
          runeIcon->setId("rune_" + bmin::toString(static_cast<int>(i)) + "_" +
                          bmin::toString(n));
          runeIcon->setPos(runeX, contentY);
          runeIcon->setScale(style.scale);
          runeIcon->setProps(SpriteElementProps{
              .width = runeIconSize,
              .height = runeIconSize,
              .spriteName = req.iconSprite,
          });
          addChild(runeIcon);
        }
        runeX += static_cast<int>(runeIconSize * style.scale) + runeGap;
      }
      runeX += groupGap - runeGap;
    }
    contentY += static_cast<int>(runeIconSize * style.scale) + vertSpacer * style.scale;
  }

  auto description = new TextParagraph(window, this);
  description->setId("description");
  description->setPos(contentX, contentY);
  description->setScale(1.f);
  TextParagraphProps descProps;
  descProps.width = contentWidth;
  descProps.fontFamily = bodyFont.fontFamily;
  descProps.fontSize = bodyFont.fontSize;
  descProps.fontColor = Colors::Black;
  descProps.textAlign = TextAlign::LEFT_TOP;
  descProps.lineSpacing = 0;
  descProps.textBlocks.pushBack({.text = props.description});
  description->setProps(descProps);
  addChild(description);
}

void PopupSpellInfo::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

MinipageCharacterSheet::MinipageCharacterSheet(sdl2w::Window* _window,
                                               UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipageCharacterSheet::setProps(const MinipageCharacterSheetProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipageCharacterSheetProps& MinipageCharacterSheet::getProps() { return props; }

const MinipageCharacterSheetProps& MinipageCharacterSheet::getProps() const {
  return props;
}

const std::pair<int, int> MinipageCharacterSheet::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipageCharacterSheet::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = bmin::makeUnique<ModalSmall>(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);

  auto title = bmin::makeUnique<TextLine>(window, modal.get());
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = "Character";
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  children.pushBack(bmin::UniquePtr<UiElement>(modal.release()));
}

void MinipageCharacterSheet::render(int dt) { UiElement::render(dt); }

} // namespace ui


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
        state::actions::adjustEquippedRune(characterPlayerId, runeType, delta),
        0);
  }

void ObserverCancelEquipRunes::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::cancelEquipRunes(), 0);
  }

void ObserverCommitEquipRunes::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::commitEquipRunes(), 0);
  }

} // namespace ui


namespace ui {

MinipageEvent::MinipageEvent(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipageEvent::setProps(const MinipageEventProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipageEventProps& MinipageEvent::getProps() { return props; }

const MinipageEventProps& MinipageEvent::getProps() const { return props; }

const std::pair<int, int> MinipageEvent::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipageEvent::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = bmin::makeUnique<ModalSmall>(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);

  auto title = bmin::makeUnique<TextLine>(window, modal.get());
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = "Event";
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title.release());

  children.pushBack(bmin::UniquePtr<UiElement>(modal.release()));
}

void MinipageEvent::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

MinipagePickUp::MinipagePickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipagePickUp::setProps(const MinipagePickUpProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipagePickUpProps& MinipagePickUp::getProps() { return props; }

const MinipagePickUpProps& MinipagePickUp::getProps() const { return props; }

const std::pair<int, int> MinipagePickUp::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipagePickUp::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
      .iconSprite = "ui_action_buttons_half_16",
      .enableCloseButton = false,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);
  addChild(modal);

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = static_cast<int>(contentW / style.scale);
  int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = props.titleText;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  scrollableSection->setPos(contentX, contentY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = unscaledContentH,
  });
  auto [scrollableContentW, scrollableContentH] = scrollableSection->getContentDims();

  auto listPickUp = new ListPickUp(window, scrollableSection);
  listPickUp->setId("listPickUp");
  listPickUp->setPos(4 * style.scale, 4 * style.scale);
  listPickUp->setScale(style.scale);

  ListPickUpProps listProps;
  listProps.width = scrollableContentW / style.scale - 8;
  if (getDatabase()) {
    for (const auto& item : props.nearbyItems) {
      const auto& itemTemplate = getDatabase()->getItemTemplate(bmin::toStringView(item.itemTemplateName));
      listProps.items.pushBack({
          .item = item,
          .itemLabel =
              itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label,
          .weight = itemTemplate.weight,
          .itemSprite = itemTemplate.iconSpriteName,
      });
    }
  }
  listPickUp->setProps(listProps);

  scrollableSection->addChild(listPickUp);
  scrollableSection->build();
  modal->addChild(scrollableSection);

  auto [buttonsW, buttonsH] = modal->getButtonsDims();
  auto [buttonsX, buttonsY] = modal->getButtonsLocation();
  const int buttonPadding = static_cast<int>(2);
  const int buttonWidth = 120;
  const int buttonHeight = ModalSmall::BUTTONS_AREA_HEIGHT;

  auto buttonGroup = new ButtonGroup(window, modal);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(buttonsX, buttonsY);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(ButtonGroupProps{
      .width = static_cast<int>(buttonsW / style.scale),
      .alignment = ButtonGroupAlignment::RIGHT,
      .buttonWidth = buttonWidth,
      .buttonHeight = buttonHeight - 2 * buttonPadding,
      .padding = buttonPadding,
      .buttons = {{//
                   .label = TRANSLATE("Done"),
                   .type = ButtonGroupButtonType::MODAL}},
  });
  if (!props.doneButtonRemoveLayerId.empty()) {
    buttonGroup->addObserverToButtonAtIndex(
        0, new ObserverRemoveLayer(props.doneButtonRemoveLayerId));
  }
  modal->addChild(buttonGroup);

  auto statusText = new TextLine(window, modal);
  statusText->setId("StatusText");
  TextFontProps statusFont;
  setBaseFontConfig(statusFont, BaseFontConfig::MODAL_TEXT);
  statusText->setPos(buttonsX + static_cast<int>(8 * style.scale),
                     buttonsY + buttonsH / 2);
  statusText->setScale(style.scale);
  statusText->setProps({
      .textBlocks =
          {
              {
                  .text = props.statusText,
              },
          },
      .fontFamily = statusFont.fontFamily,
      .fontSize = statusFont.fontSize,
      .fontColor = Colors::DarkGrey,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  modal->addChild(statusText);

  if (!props.partyMemberSprites.empty()) {
    auto partySelector = new PartyMemberIconSelector(window, this);
    partySelector->setId("partyMemberSelector");
    partySelector->setScale(style.scale);
    partySelector->setProps(PartyMemberIconSelectorProps{
        .members = props.partyMemberSprites,
        .selectedIndex = props.partyMemberIndex,
        .target = PartyMemberIconSelectorTarget::PICKUP,
    });
    auto [selectorW, selectorH] = partySelector->getDims();
    partySelector->setPos(contentX + contentW - selectorW - static_cast<int>(4 * style.scale),
                          style.y + static_cast<int>(8 * style.scale));
    partySelector->build();

    auto weightText = new TextLine(window, this);
    weightText->setId("weightText");
    TextFontProps weightFont;
    setBaseFontConfig(weightFont, BaseFontConfig::MODAL_TEXT);
    auto [selectorX, selectorY] = partySelector->getPos();
    weightText->setPos(selectorX + selectorW / 2,
                       selectorY + selectorH + static_cast<int>(16 * style.scale));
    weightText->setScale(1.f);
    weightText->setProps({
        .textBlocks =
            {
                {
                    .text = props.weightText,
                },
            },
        .fontFamily = weightFont.fontFamily,
        .fontSize = weightFont.fontSize,
        .fontColor = Colors::DarkGrey,
        .textAlign = TextAlign::CENTER,
    });

    addChild(partySelector);
    addChild(weightText);
  }
}

void MinipagePickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

MinipageSpellCast::MinipageSpellCast(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void MinipageSpellCast::setProps(const MinipageSpellCastProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipageSpellCastProps& MinipageSpellCast::getProps() { return props; }

const MinipageSpellCastProps& MinipageSpellCast::getProps() const { return props; }

const std::pair<int, int> MinipageSpellCast::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipageSpellCast::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
      .iconSprite = "ui_action_buttons_half_16",
      .enableCloseButton = false,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);
  addChild(modal);

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = static_cast<int>(contentW / style.scale);
  int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = props.titleText;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  scrollableSection->setPos(contentX, contentY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = unscaledContentH,
  });
  auto [scrollableContentW, scrollableContentH] = scrollableSection->getContentDims();
  (void)scrollableContentH;

  auto listSpells = new ListMagicSpells(window, scrollableSection);
  listSpells->setId("listSpells");
  listSpells->setPos(4 * style.scale, 4 * style.scale);
  listSpells->setScale(style.scale);

  ListMagicSpellsProps listProps;
  listProps.casterId = props.casterId;
  listProps.width = scrollableContentW / style.scale - 8;
  listProps.enableSpellInfoOnClick = false;
  listProps.enableSpellCastOnClick = true;
  for (const auto& spell : props.spells) {
    listProps.spells.pushBack(ListMagicSpellsPropsSpell{
        .id = spell.id,
        .label = spell.label,
        .iconSprite = spell.iconSprite,
        .requiredRuneSprites = spell.requiredRuneSprites,
    });
  }
  listSpells->setProps(listProps);

  scrollableSection->addChild(listSpells);
  scrollableSection->build();
  modal->addChild(scrollableSection);

  auto [buttonsW, buttonsH] = modal->getButtonsDims();
  auto [buttonsX, buttonsY] = modal->getButtonsLocation();
  const int buttonPadding = static_cast<int>(2);
  const int buttonWidth = 120;
  const int buttonHeight = ModalSmall::BUTTONS_AREA_HEIGHT;

  auto buttonGroup = new ButtonGroup(window, modal);
  buttonGroup->setId("buttonGroup");
  buttonGroup->setPos(buttonsX, buttonsY);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(ButtonGroupProps{
      .width = static_cast<int>(buttonsW / style.scale),
      .alignment = ButtonGroupAlignment::RIGHT,
      .buttonWidth = buttonWidth,
      .buttonHeight = buttonHeight - 2 * buttonPadding,
      .padding = buttonPadding,
      .buttons = {{//
                   .label = TRANSLATE("Done"),
                   .type = ButtonGroupButtonType::MODAL}},
  });
  if (!props.doneButtonRemoveLayerId.empty()) {
    buttonGroup->addObserverToButtonAtIndex(
        0, new ObserverRemoveLayer(props.doneButtonRemoveLayerId));
  }
  modal->addChild(buttonGroup);

  auto statusText = new TextLine(window, modal);
  statusText->setId("StatusText");
  TextFontProps statusFont;
  setBaseFontConfig(statusFont, BaseFontConfig::MODAL_TEXT);
  statusText->setPos(buttonsX + static_cast<int>(8 * style.scale),
                     buttonsY + buttonsH / 2);
  statusText->setScale(style.scale);
  statusText->setProps({
      .textBlocks =
          {
              {
                  .text = props.statusText,
              },
          },
      .fontFamily = statusFont.fontFamily,
      .fontSize = statusFont.fontSize,
      .fontColor = Colors::DarkGrey,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  modal->addChild(statusText);
}

void MinipageSpellCast::render(int dt) { UiElement::render(dt); }

} // namespace ui
