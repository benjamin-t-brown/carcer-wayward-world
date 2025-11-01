#include "ListInventory.h"
#include "model/Items.h"
#include "state/actions/ui/UIShowContextMenuInventory.hpp"
#include "ui/colors.h"
#include "ui/components/VerticalList.h"
#include "ui/elements/ButtonModal.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"

namespace ui {

class ListInventoryItemContextButtonObserver : public UiEventObserver,
                                               public state::StateManagerInterface {
public:
  sdl2w::Window* window;
  std::string itemName;
  std::string itemId;

  ListInventoryItemContextButtonObserver(sdl2w::Window* _window,
                                         std::string _itemName,
                                         std::string _itemId)
      : window(_window), itemName(_itemName), itemId(_itemId) {}

  void onClick(int x, int y, int button) override {
    LOG(INFO) << "ListInventoryItemContextButtonObserver::onClick"
              << " itemName: " << itemName << " itemId: " << itemId << LOG_ENDL;
    getStateManager()->enqueueAction(
        getStateManager()->getState(),
        new state::actions::UIShowContextMenuInventory(window, itemName, itemId),
        0);
  }
};

// ListInventory Implementation
ListInventory::ListInventory(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListInventory::getDims() const {
  if (children.empty()) {
    return {style.width, 0};
  }

  const auto& list = children[0];

  return list->getDims();
}

void ListInventory::setProps(const ListInventoryProps& _props) {
  props = _props;
  build();
}

const ListInventoryProps& ListInventory::getProps() const { return props; }

void ListInventory::build() {
  children.clear();

  if (props.character == nullptr) {
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
  listProps.lineHeight = 32; // Height for each inventory item
  listProps.lineGap = 8;
  listProps.bgColor = Colors::White;
  list->setProps(listProps);

  std::vector<UiElement*> itemElementsToAdd;

  // Add ListInventoryItem children for each item
  for (int i = 0; i < static_cast<int>(props.character->inventory.size()); i++) {
    const auto& chInvItem = props.character->inventory[i];
    const auto& itemTemplate = getDatabase()->getItemTemplate(chInvItem.itemName);
    auto itemElement = new ListInventoryItem(window);
    itemElement->setId("item" + std::to_string(i));
    itemElement->setProps(
        ListInventoryItemProps{&itemTemplate, chInvItem.id, chInvItem.quantity});
    itemElementsToAdd.push_back(itemElement);
  }
  list->addListItems(itemElementsToAdd);

  children.push_back(std::move(list));
}

void ListInventory::render(int dt) {
  //
  UiElement::render(dt);
}

// ListInventoryItem Implementation
ListInventoryItem::ListInventoryItem(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  style.height = 32;
}

void ListInventoryItem::setProps(const ListInventoryItemProps& _props) {
  props = _props;
  build();
}

const ListInventoryItemProps& ListInventoryItem::getProps() const { return props; }

void ListInventoryItem::build() {
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

  // Create context button
  auto contextBtn = std::make_unique<ButtonModal>(window, this);
  contextBtn->setId("contextBtn");
  BaseStyle buttonStyle = contextBtn->getStyle();
  buttonStyle.x = style.x + style.width - 32; // Right side
  buttonStyle.y = style.y + (listItemHeight - 32) / 2;
  buttonStyle.width = 32;
  buttonStyle.height = 32;
  buttonStyle.scale = 1.0f;
  contextBtn->setStyle(buttonStyle);

  ButtonModalProps buttonProps;
  buttonProps.text = "*"; // Context menu indicator
  buttonProps.isSelected = false;
  contextBtn->setProps(buttonProps);
  contextBtn->addEventObserver(std::unique_ptr<UiEventObserver>(
      //
      new ListInventoryItemContextButtonObserver(
          window, itemTemplate.name, props.itemId)));
  children.push_back(std::move(contextBtn));
}

void ListInventoryItem::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, bgColor);
  UiElement::render(dt);
}

} // namespace ui
