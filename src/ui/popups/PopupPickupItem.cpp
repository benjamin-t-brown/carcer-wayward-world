#include "PopupPickupItem.h"
#include "sdl2w/Logger.h"
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
  closeButton->addEventObserver(new PopupPickupItemCloseButtonObserver(layer));
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
