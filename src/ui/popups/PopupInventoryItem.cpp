#include "PopupInventoryItem.h"
#include "layers/ui/LayerInventoryContext.h"
#include "lib/sdl2w/L10n.h"
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
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  borderStyle.scale = style.scale;
  border->setProps(BorderDropShadowProps{});
  addChild(border);

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  auto& closeStyle = closeButton->getStyle();
  closeStyle.x =
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale;
  closeStyle.y = style.y + padding * style.scale;
  closeStyle.width = closeButtonSize;
  closeStyle.height = closeButtonSize;
  closeStyle.scale = style.scale;
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
    auto& buttonStyle = button->getStyle();
    buttonStyle.x = style.x + _x;
    buttonStyle.y = style.y + _y;
    buttonStyle.width = actionButtonsWidth;
    buttonStyle.height = actionButtonsHeight;
    buttonStyle.scale = style.scale;
    button->setProps(ButtonModalProps{.text = text});
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
  auto& spriteBgStyle = spriteBgQuad->getStyle();
  spriteBgStyle.x = style.x + padding * style.scale;
  spriteBgStyle.y = style.y + padding * style.scale;
  spriteBgStyle.width = iconBgSize;
  spriteBgStyle.height = iconBgSize;
  spriteBgStyle.scale = style.scale;
  spriteBgQuad->setProps(QuadProps{.bgColor = Colors::LightGrey});
  addChild(spriteBgQuad);

  auto spriteQuad = new Quad(window, this);
  spriteQuad->setId("icon");
  auto& spriteStyle = spriteQuad->getStyle();
  spriteStyle.x = iconBgSize / 2.f - (itemIconSize / 2.f) * 2.f * style.scale;
  spriteStyle.y = iconBgSize / 2.f - (itemIconSize / 2.f) * 2.f * style.scale;
  spriteStyle.width = itemIconSize;
  spriteStyle.height = itemIconSize;
  spriteStyle.scale = 2.f * style.scale;
  spriteQuad->setProps(QuadProps{.bgSprite = props.spriteName});
  spriteBgQuad->addChild(spriteQuad);

  auto label = new TextLine(window, this);
  label->setId("label");
  auto& labelStyle = label->getStyle();
  setBaseFontConfig(labelStyle, BaseFontConfig::MODAL_TITLE);
  labelStyle.x =
      style.x + padding * style.scale + iconBgSize * style.scale + padding * style.scale;
  labelStyle.y = style.y + padding * style.scale + (iconBgSize * style.scale) / 2;
  labelStyle.scale = 1.f;
  labelStyle.fontColor = Colors::Black;
  labelStyle.textAlign = TextAlign::LEFT_CENTER;
  label->setProps(TextLineProps{
      .textBlocks =
          {
              {
                  .text = props.label,
              },
          },
  });
  addChild(label);

  auto itemInfo = new ItemInfo(window, this);
  itemInfo->setId("itemInfo");
  auto& itemInfoStyle = itemInfo->getStyle();
  itemInfoStyle.x = style.x + padding * style.scale;
  itemInfoStyle.y = style.y + padding * style.scale + iconBgSize * style.scale +
                    vertSpacerHeight * style.scale;
  itemInfoStyle.width = static_cast<int>(
      (scaledWidth - padding * 2 * style.scale - actionButtonsWidth * style.scale -
       8 * style.scale) /
      style.scale);
  itemInfoStyle.scale = 1.f;
  itemInfo->setProps(ItemInfoProps{
      .description = props.description,
      .weight = props.weight,
      .value = props.value,
  });
  addChild(itemInfo);
}

void PopupInventoryItem::render(int dt) { UiElement::render(dt); }

} // namespace ui
