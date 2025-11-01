#include "ListPickUp.h"
#include "model/Items.h"
#include "ui/colors.h"
#include "ui/components/VerticalList.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"

namespace ui {

// ListPickUp Implementation
ListPickUp::ListPickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListPickUp::getDims() const {
  if (children.empty()) {
    return {style.width, 0};
  }

  const auto& list = children[0];

  return list->getDims();
}

void ListPickUp::setProps(const ListPickUpProps& _props) {
  props = _props;
  build();
}

const ListPickUpProps& ListPickUp::getProps() const { return props; }

void ListPickUp::build() {
  children.clear();

  if (props.itemNames.empty()) {
    return;
  }

  // Create VerticalList component
  auto list = std::make_unique<VerticalList>(window, this);
  list->setId("list");
  BaseStyle listStyle = list->getStyle();
  listStyle.x = style.x;
  listStyle.y = style.y;
  listStyle.width = style.width;
  listStyle.scale = 1.0f;
  list->setStyle(listStyle);

  // Configure VerticalList properties
  VerticalListProps listProps;
  listProps.lineHeight = 32; // Height for each pickup item
  listProps.lineGap = 8;
  listProps.bgColor = Colors::White;
  list->setProps(listProps);

  std::vector<UiElement*> itemElementsToAdd;

  // Add ListPickUpItem children for each item
  for (int i = 0; i < static_cast<int>(props.itemNames.size()); i++) {
    const auto& itemName = props.itemNames[i];
    const auto& itemTemplate = getDatabase()->getItemTemplate(itemName);
    auto itemElement = new ListPickUpItem(window);
    itemElement->setId("item" + std::to_string(i));
    itemElement->setProps(ListPickUpItemProps{&itemTemplate, itemName});
    itemElementsToAdd.push_back(itemElement);
  }
  list->addListItems(itemElementsToAdd);

  children.push_back(std::move(list));
}

void ListPickUp::render(int dt) {
  //
  UiElement::render(dt);
}

// ListPickUpItem Implementation
ListPickUpItem::ListPickUpItem(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  style.height = 32;
}

void ListPickUpItem::setProps(const ListPickUpItemProps& _props) {
  props = _props;
  build();
}

const ListPickUpItemProps& ListPickUpItem::getProps() const { return props; }

void ListPickUpItem::build() {
  children.clear();

  if (props.itemTemplate == nullptr) {
    return;
  }

  const auto& itemTemplate = *props.itemTemplate;
  const int listItemHeight = 28;

  // Create icon element
  auto icon = std::make_unique<SpriteElement>(window, this);
  icon->setId("icon");
  int iconHeight = 32;
  BaseStyle iconStyle;
  iconStyle.x = style.x;
  iconStyle.y = style.y + (listItemHeight - iconHeight) / 2;
  iconStyle.width = 16;
  iconStyle.height = 16;
  iconStyle.scale = 2.0f;
  icon->setStyle(iconStyle);
  icon->setSprite(itemTemplate.iconSpriteName);
  children.push_back(std::move(icon));

  // Create label element
  auto label = std::make_unique<TextLine>(window, this);
  label->setId("label");
  BaseStyle labelStyle;
  labelStyle.x = style.x + 32 + 8;
  labelStyle.fontFamily = FontFamily::PARAGRAPH;
  labelStyle.fontSize = sdl2w::TEXT_SIZE_16;
  labelStyle.fontColor = Colors::Black;
  labelStyle.textAlign = TextAlign::LEFT_TOP;
  label->setStyle(labelStyle);
  // Set label text
  TextLineProps labelProps;
  TextBlock labelBlock;
  labelBlock.text = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  labelProps.textBlocks.push_back(labelBlock);
  label->setProps(labelProps);
  labelStyle.y = style.y + (listItemHeight - label->getDims().second) / 2;
  label->setStyle(labelStyle);
  children.push_back(std::move(label));

  // Create weight text element (replaces context button)
  auto weightText = std::make_unique<TextLine>(window, this);
  weightText->setId("weightText");
  BaseStyle weightStyle;
  weightStyle.fontFamily = FontFamily::PARAGRAPH;
  weightStyle.fontSize = sdl2w::TEXT_SIZE_16;
  weightStyle.fontColor = Colors::Black;
  weightStyle.textAlign = TextAlign::LEFT_TOP;
  // Set weight text first to get dimensions
  TextLineProps weightProps;
  TextBlock weightBlock;
  weightBlock.text = std::to_string(itemTemplate.weight) + " lbs";
  weightProps.textBlocks.push_back(weightBlock);
  weightText->setProps(weightProps);
  weightStyle.x = style.x + style.width - weightText->getDims().first - 8; // Right side with padding
  weightStyle.y = style.y + (listItemHeight - weightText->getDims().second) / 2;
  weightText->setStyle(weightStyle);
  children.push_back(std::move(weightText));
}

void ListPickUpItem::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, bgColor);
  UiElement::render(dt);
}

} // namespace ui

