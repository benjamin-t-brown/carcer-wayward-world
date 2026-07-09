#include "PopupInventoryItem.h"
#include "layers/ui/LayerInventoryContext.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/ItemInfo.h"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include "ui/observers/ObserverShowLayerDropContext.hpp"
#include "ui/observers/ObserverShowLayerGiveContext.hpp"

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
      new ObserverRemoveLayer(layers::LayerInventoryContext::LAYER_ID));
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
