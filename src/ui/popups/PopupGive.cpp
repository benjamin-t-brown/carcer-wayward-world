#include "PopupGive.h"
#include "layers/ui/LayerGiveContext.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/elements/VerticalList.h"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/elements/HorizontalSlider.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ObserverGiveInventoryItem.hpp"
#include "ui/observers/ObserverRemoveLayer.hpp"

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
      new ObserverRemoveLayer(layers::LayerGiveContext::LAYER_ID));
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
