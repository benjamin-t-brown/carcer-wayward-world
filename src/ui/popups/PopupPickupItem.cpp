#include "PopupPickupItem.h"
#include "lib/sdl2w/Logger.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/colors.h"
#include "ui/components/ItemInfo.h"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"

namespace ui {

class PopupPickupItemCloseButtonObserver : public UiEventObserver,
                                           public state::StateManagerInterface {

  layers::Layer* layer;

public:
  PopupPickupItemCloseButtonObserver(layers::Layer* _layer) : layer(_layer) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "PopupPickupItemCloseButtonObserver::onClick" << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiRemoveLayer(layer->getId()), 0);
  }
};

PopupPickupItem::PopupPickupItem(sdl2w::Window* _window,
                                 layers::Layer* _layer,
                                 PopupOrientation _orientation)
    : UiElement(_window, nullptr) {
  this->layer = _layer;
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
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  borderStyle.scale = style.scale;
  border->setProps(BorderDropShadowProps{
      .backgroundColor = Colors::White,
      .shadowColor = Colors::Black,
      .shadowOffsetX = -8,
      .shadowOffsetY = 8,
      .borderSize = 2,
  });
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
  closeButton->addEventObserver(new PopupPickupItemCloseButtonObserver(layer));
  addChild(closeButton);

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
  itemInfoStyle.width =
      static_cast<int>((scaledWidth - padding * 2 * style.scale - 8 * style.scale) / style.scale);
  itemInfoStyle.scale = style.scale;
  itemInfo->setProps(ItemInfoProps{
      .description = props.description,
      .weight = props.weight,
      .value = props.value,
  });
  addChild(itemInfo);
}

void PopupPickupItem::render(int dt) { UiElement::render(dt); }

} // namespace ui
