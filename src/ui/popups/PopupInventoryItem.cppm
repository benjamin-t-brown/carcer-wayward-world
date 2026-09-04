module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.popups.PopupInventoryItem;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
import carcer.actions.ui.UiShowLayerDropContext;
import carcer.actions.ui.UiShowLayerGiveContext;
import carcer.ui.ObserverRemoveLayer;
import carcer.state;
import carcer.ui.BorderDropShadow;
import carcer.ui.elements;
import carcer.ui.core;
import carcer.ui.components.ItemInfo;
#include "macros.h"

export {

// --- from ui/popups/PopupInventoryItem.h ---
namespace ui {

enum PopupOrientation { NARROW, WIDE };

struct PopupInventoryItemProps {
  bmin::String characterPlayerId;
  model::ItemInstance item;
  bmin::String label;
  bmin::String description;
  bmin::String spriteName;
  int weight = 0;
  int value = 0;
  bool usable = false;
  bool equippable = false;
  PopupOrientation orientation = WIDE;
};

class PopupInventoryItem : public UiElement {
  PopupInventoryItemProps props;

public:
  PopupInventoryItem(sdl2w::Window* _window,
                     UiElement* _parent,
                     PopupOrientation _orientation = WIDE);

  void setProps(const PopupInventoryItemProps& _props);
  PopupInventoryItemProps& getProps();
  const PopupInventoryItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerDropContext.hpp ---
namespace ui {

class ObserverShowLayerDropContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverShowLayerDropContext(sdl2w::Window* _window,
                               const bmin::String& _characterPlayerId,
                               const bmin::String& _itemId)
      : window(_window),
        characterPlayerId(_characterPlayerId),
        itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverShowLayerGiveContext.hpp ---
namespace ui {

class ObserverShowLayerGiveContext : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

public:
  ObserverShowLayerGiveContext(sdl2w::Window* _window,
                               const bmin::String& _fromCharacterPlayerId,
                               const bmin::String& _itemId)
      : window(_window),
        fromCharacterPlayerId(_fromCharacterPlayerId),
        itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

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
        new state::actions::UiShowLayerDropContext(window, characterPlayerId, itemId),
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
        new state::actions::UiShowLayerGiveContext(
            window, fromCharacterPlayerId, itemId),
        0);
  }

} // namespace ui
