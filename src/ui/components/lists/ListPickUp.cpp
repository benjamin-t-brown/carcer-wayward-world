#include "ListPickUp.h"
#include "../VerticalList.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/observers/ObserverPickUpItem.hpp"
#include "ui/observers/ObserverShowLayerPickUpContext.hpp"
#include <algorithm>

namespace ui {

ListPickUp::ListPickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListPickUp::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListPickUp::setProps(const ListPickUpProps& _props) {
  props = _props;
  build();
}

const ListPickUpProps& ListPickUp::getProps() const { return props; }

UiElement* ListPickUp::createItemElement(const ListPickUpPropsItem& listItem) {
  auto container = new Quad(window, this);
  container->setId(listItem.item.itemName);
  auto& containerStyle = container->getStyle();
  containerStyle.width = style.width * style.scale;
  containerStyle.height = props.lineHeight * style.scale;
  containerStyle.scale = 1.0f;
  container->setProps({
      .bgColor = Colors::Transparent,
  });

  float iconScale = 2.f;
  auto icon = new Quad(window, this);
  icon->setId("icon");
  auto& iconStyle = icon->getStyle();
  iconStyle.width = iconSpriteSize;
  iconStyle.height = iconSpriteSize;
  iconStyle.x = 0;
  iconStyle.y = (containerStyle.height - iconStyle.height * style.scale * iconScale) / 2;
  iconStyle.scale = iconScale * style.scale;
  icon->setProps({
      .bgColor = {66, 202, 253, 50},
      .bgSprite = listItem.itemSprite,
  });
  container->addChild(icon);

  const int weightRightPadding = 8;
  const int labelWeightGap = 8;
  const int labelX = iconStyle.width + static_cast<int>(24 * style.scale);
  const int scaledContextBtnSize = contextBtnSize * style.scale;

  auto contextBtn = new ButtonModal(window, this);
  contextBtn->setId("contextBtn");
  auto& contextBtnStyle = contextBtn->getStyle();
  contextBtnStyle.x = containerStyle.width - scaledContextBtnSize;
  contextBtnStyle.y = (containerStyle.height - scaledContextBtnSize) / 2;
  contextBtnStyle.width = scaledContextBtnSize;
  contextBtnStyle.height = scaledContextBtnSize;
  contextBtnStyle.fontColor = Colors::DarkBlue;
  contextBtnStyle.scale = 1.0f;
  contextBtn->setProps({
      .text = "?",
      .bgColor = Colors::Transparent,
      .bgColorTopRight = Colors::Transparent,
      .bgColorBottomLeft = Colors::Transparent,
  });
  contextBtn->addEventObserver(
      new ui::ObserverShowLayerPickUpContext(window, listItem.item));
  container->addChild(contextBtn);

  auto weightText = new TextLine(window, this);
  weightText->setId("weightText");
  auto& weightStyle = weightText->getStyle();
  setBaseFontConfig(weightStyle, BaseFontConfig::MODAL_TEXT);
  weightStyle.fontColor = Colors::DarkGrey;
  weightStyle.textAlign = TextAlign::LEFT_CENTER;
  weightStyle.scale = 1.0f;
  weightStyle.y = containerStyle.height / 2;
  weightText->setProps({
      .textBlocks =
          {
              {
                  .text =
                      std::to_string(listItem.item.quantity * listItem.weight) + " lbs",
              },
          },
  });
  auto [weightWidth, _] = weightText->getDims();
  weightStyle.x = contextBtnStyle.x - weightWidth - weightRightPadding;
  weightText->build();
  container->addChild(weightText);

  auto label = new ButtonTextWrap(window, this);
  label->setId("label");
  auto& labelStyle = label->getStyle();
  setBaseFontConfig(labelStyle, BaseFontConfig::MODAL_TEXT);
  labelStyle.fontSize = sdl2w::TEXT_SIZE_18;
  labelStyle.fontColor = Colors::Black;
  labelStyle.scale = 1.0f;
  labelStyle.x = labelX;
  labelStyle.y = 0;
  labelStyle.width =
      static_cast<int>((weightStyle.x - labelX - labelWeightGap) / labelStyle.scale);
  label->setStyle(labelStyle);
  label->setProps({
      .text = listItem.item.itemName,
  });
  const int textOnlyHeight = label->getDims().second;
  const int verticalPadding =
      std::max(0,
               static_cast<int>((containerStyle.height - textOnlyHeight) /
                                (2 * labelStyle.scale)));
  label->setProps({
      .text = listItem.itemLabel,
      .verticalPadding = verticalPadding,
  });
  label->addEventObserver(new ui::ObserverPickUpItem(listItem.item));
  label->build();
  container->addChild(label);

  return container;
}

void ListPickUp::build() {
  children.clear();

  if (props.items.empty()) {
    return;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  auto& listStyle = list->getStyle();
  listStyle.x = style.x;
  listStyle.y = style.y + static_cast<int>(props.paddingTop * style.scale);
  listStyle.width = style.width * style.scale;
  listStyle.scale = 1.0f;

  for (const auto& item : props.items) {
    list->addChild(createItemElement(item));
  }

  list->setProps({
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListPickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui
