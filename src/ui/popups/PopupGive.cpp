#include "PopupGive.h"
#include "layers/ui/LayerGiveContext.h"
#include "lib/sdl2w/L10n.h"
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
  auto& closeStyle = closeButton->getStyle();
  closeStyle.x =
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale;
  closeStyle.y = style.y + padding * style.scale;
  closeStyle.width = closeButtonSize;
  closeStyle.height = closeButtonSize;
  closeStyle.scale = style.scale;
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(
      new ObserverRemoveLayer(layers::LayerGiveContext::LAYER_ID));
  addChild(closeButton);

  auto title = new TextLine(window, this);
  title->setId("title");
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.x = style.x + padding * style.scale;
  titleStyle.y = style.y + padding * style.scale + (closeButtonSize * style.scale) / 2;
  titleStyle.scale = 1.f;
  titleStyle.fontColor = Colors::Black;
  titleStyle.textAlign = TextAlign::LEFT_CENTER;
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
  });
  addChild(title);
  auto [titleWidth, titleHeight] = title->calculateTextDims();

  int contentY = style.y + paddingScaled + titleHeight + paddingScaled * 2;

  if (props.showQuantitySlider) {
    auto slider = new HorizontalSlider(window, this);
    slider->setId("quantitySlider");
    quantitySlider = slider;
    auto& sliderStyle = slider->getStyle();
    sliderStyle.x = style.x + padding * style.scale;
    sliderStyle.y = contentY;
    sliderStyle.scale = style.scale;
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
  auto& listStyle = list->getStyle();
  listStyle.x = style.x + padding * style.scale;
  listStyle.y = contentY;
  listStyle.width = contentWidth;
  listStyle.scale = style.scale;

  const int iconSize = 32;
  const int rowHeight = 32;
  const int rowGap = 2;
  const int rowWidth = style.width - 2 * padding;
  const int buttonWidth = rowWidth - iconSize;
  for (const auto& member : props.partyMembers) {
    auto row = new Quad(window, list);
    row->setId("row-" + member.characterPlayerId);
    auto& rowStyle = row->getStyle();
    rowStyle.width = rowWidth;
    rowStyle.height = rowHeight;
    rowStyle.scale = style.scale;
    row->setProps(QuadProps{.bgColor = Colors::Transparent});

    auto button = new ButtonModal(window, row);
    button->setId("btn-" + member.characterPlayerId);
    auto& buttonStyle = button->getStyle();
    buttonStyle.x = iconSize;
    buttonStyle.y = 0;
    buttonStyle.width = buttonWidth;
    buttonStyle.height = rowHeight;
    buttonStyle.scale = style.scale;
    button->setProps(ButtonModalProps{
        .text = member.label,
    });
    button->addEventObserver(new ObserverGiveInventoryItem(
        member.characterPlayerId, props.fromCharacterPlayerId, props.itemId, this));
    row->addChild(button);

    auto iconBg = new OutsetRectangle(window, row);
    iconBg->setId("iconBg-" + member.characterPlayerId);
    auto& iconBgStyle = iconBg->getStyle();
    iconBgStyle.x = 0;
    iconBgStyle.y = 0;
    iconBgStyle.width = iconSize;
    iconBgStyle.height = iconSize;
    iconBgStyle.scale = style.scale;
    iconBg->setProps(OutsetRectangleProps{
        .color = Colors::LightGrey,
        .colorTopRight = Colors::White,
        .colorBottomLeft = Colors::ButtonModalGrey2,
        .borderSize = 0,
    });
    row->addChild(iconBg);

    auto sprite = new Quad(window, iconBg);
    sprite->setId("sprite-" + member.characterPlayerId);
    auto& spriteStyle = sprite->getStyle();
    spriteStyle.x = 0;
    spriteStyle.y = 0;
    spriteStyle.width = iconSize;
    spriteStyle.height = iconSize;
    spriteStyle.scale = style.scale;
    sprite->setProps(QuadProps{.bgSprite = member.spriteName});
    iconBg->addChild(sprite);

    list->addListItem(row);
  }

  list->setProps({
      .lineHeight = static_cast<int>((rowHeight + rowGap) * style.scale),
      .lineGap = rowGap,
      .bgColor = Colors::Transparent,
  });
  auto [listW, listH] = list->getDims();
  addChild(list);

  style.height = ((contentY + listH + paddingScaled) - style.y) / style.scale;

  auto border = new BorderDropShadow(window, this);
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  borderStyle.scale = style.scale;
  border->setProps(BorderDropShadowProps{});
  children.insert(children.begin(), bmin::UniquePtr<UiElement>(border));
}

void PopupGive::render(int dt) { UiElement::render(dt); }

} // namespace ui
