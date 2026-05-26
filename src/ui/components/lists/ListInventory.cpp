#include "ListInventory.h"
#include "../VerticalList.h"
#include "ui/colors.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ObserverInventorySelectItem.hpp"
#include "ui/observers/ObserverShowLayerInventoryContext.hpp"

namespace ui {

// ListInventory Implementation
ListInventory::ListInventory(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListInventory::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListInventory::setProps(const ListInventoryProps& _props) {
  props = _props;
  build();
}

const ListInventoryProps& ListInventory::getProps() const { return props; }

UiElement* ListInventory::createItemElement(const ListInventoryPropsItem& item) {
  auto container = new Quad(window, this);
  container->setId(item.itemName);
  auto& s = container->getStyle();
  s.width = style.width * style.scale;
  s.height = props.lineHeight * style.scale;
  s.scale = 1.0f;
  container->setProps({
      .bgColor = Colors::Transparent,
  });

  // Create icon element
  float iconScale = 2.f;
  auto icon = new Quad(window, this);
  icon->setId("icon");
  auto& iconStyle = icon->getStyle();
  iconStyle.width = iconSpriteSize;
  iconStyle.height = iconSpriteSize;
  iconStyle.x = 0;
  iconStyle.y = (s.height - iconStyle.height * style.scale * iconScale) / 2;
  iconStyle.scale = iconScale * style.scale;
  icon->setProps({
      .bgColor = {66, 202, 253, 50},
      .bgSprite = item.itemSprite,
  });
  container->addChild(icon);

  // Create label element
  auto label = new TextLine(window, this);
  label->setId("label");
  auto& labelStyle = label->getStyle();
  setBaseFontConfig(labelStyle, BaseFontConfig::MODAL_TEXT);
  labelStyle.fontSize = sdl2w::TEXT_SIZE_18;
  labelStyle.fontColor = Colors::Black;
  labelStyle.textAlign = TextAlign::LEFT_CENTER;
  labelStyle.scale = 1.0f;
  labelStyle.x = iconStyle.width + 24 * style.scale;
  labelStyle.y = (s.height / 2);
  label->addEventObserver(
      new ui::ObserverInventorySelectItem(item.itemName, item.itemId));

  label->setProps({
      .textBlocks =
          {
              {
                  .text = item.itemLabel,
              },
          },
  });
  container->addChild(label);

  // Create context button
  const int contextBtnSize = 32 * style.scale;
  auto contextBtn = new ButtonModal(window, this);
  contextBtn->setId("contextBtn");
  auto& contextBtnStyle = contextBtn->getStyle();
  contextBtnStyle.x = s.width - contextBtnSize;
  contextBtnStyle.y = (s.height - contextBtnSize) / 2;
  contextBtnStyle.width = contextBtnSize;
  contextBtnStyle.height = contextBtnSize;
  contextBtnStyle.scale = 1.0f;
  contextBtn->setProps({
      .text = "*",
  });
  contextBtn->addEventObserver(
      new ui::ObserverShowLayerInventoryContext(window, item.itemName, item.itemId));
  container->addChild(contextBtn);

  return container;
}

void ListInventory::build() {
  children.clear();

  // Create VerticalList component
  auto list = new VerticalList(window, this);
  list->setId("list");
  auto& s = list->getStyle();
  s.x = style.x;
  s.y = style.y + static_cast<int>(props.paddingTop * style.scale);
  s.width = style.width * style.scale;
  s.scale = 1.0f;

  for (const auto& item : props.items) {
    list->addChild(createItemElement(item));
  }

  list->setProps({
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = props.lineGap,
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListInventory::render(int dt) {
  //
  UiElement::render(dt);
}

} // namespace ui
