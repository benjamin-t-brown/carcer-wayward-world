#include "ListInventory.h"
#include "../VerticalList.h"
#include "model/Items.h"
#include "state/actions/ui/UiShowContextMenuInventory.hpp"
#include "ui/colors.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonModal.h"

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
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    LOG(INFO) << "ListInventoryItemContextButtonObserver::onClick"
              << " itemName: " << itemName << " itemId: " << itemId << LOG_ENDL;
    stateManager->enqueueAction(
        stateManager->getState(),
        new state::actions::UiShowContextMenuInventory(window, itemName, itemId),
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

UiElement* ListInventory::createItemElement(const ListInventoryPropsItem& item) {
  auto container = new Quad(window, this);
  container->setId(item.itemName);
  auto& s = container->getStyle();
  s.width = style.width * style.scale;
  s.height = props.lineHeight * style.scale;
  s.scale = 1.0f;
  container->setProps({
      .bgColor = Colors::White,
  });

  // Create icon element
  auto icon = new Quad(window, this);
  icon->setId("icon");
  auto& iconStyle = icon->getStyle();
  iconStyle.width = iconSpriteSize * style.scale;
  iconStyle.height = iconSpriteSize * style.scale;
  iconStyle.x = 0;
  iconStyle.y = (s.height - iconStyle.height) / 2;
  iconStyle.scale = 1.0f;
  icon->setProps({
      .bgSprite = item.itemSprite,
  });
  container->addChild(icon);

  // Create label element
  auto label = new TextLine(window, this);
  label->setId("label");
  auto& labelStyle = label->getStyle();
  labelStyle.fontSize = sdl2w::TEXT_SIZE_18;
  labelStyle.fontColor = Colors::Black;
  labelStyle.fontFamily = FontFamily::PARAGRAPH;
  labelStyle.textAlign = TextAlign::LEFT_CENTER;
  labelStyle.scale = 1.0f;
  labelStyle.x = iconStyle.width + 8 * style.scale;
  labelStyle.y = (s.height / 2);

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
  s.y = style.y;
  s.width = style.width * style.scale;
  s.scale = 1.0f;

  for (const auto& item : props.items) {
    list->addChild(createItemElement(item));
  }

  list->setProps({
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = props.lineGap,
      .bgColor = Colors::White,
  });

  addChild(list);
}

void ListInventory::render(int dt) {
  //
  UiElement::render(dt);
}

// // ListInventoryItem Implementation
// ListInventoryItem::ListInventoryItem(sdl2w::Window* _window, UiElement* _parent)
//     : UiElement(_window, _parent) {
//   style.height = 32;
// }

// void ListInventoryItem::setProps(const ListInventoryItemProps& _props) {
//   props = _props;
//   build();
// }

// const ListInventoryItemProps& ListInventoryItem::getProps() const { return props; }

// void ListInventoryItem::build() {
//   children.clear();

//   if (props.itemTemplate == nullptr) {
//     return;
//   }

//   const auto& itemTemplate = *props.itemTemplate;
//   const int listItemHeight = 28;

//   // Create icon element
//   auto icon = std::make_unique<SpriteElement>(window, this);
//   icon->setId("icon");
//   int iconHeight = 32;
//   BaseStyle iconStyle;
//   iconStyle.x = style.x;
//   iconStyle.y = style.y + (listItemHeight - iconHeight) / 2;
//   iconStyle.width = 16;
//   iconStyle.height = 16;
//   iconStyle.scale = 2.0f;
//   icon->setStyle(iconStyle);
//   icon->setSprite(itemTemplate.iconSpriteName);
//   children.push_back(std::move(icon));

//   // Create label element
//   auto label = std::make_unique<TextLine>(window, this);
//   label->setId("label");
//   BaseStyle labelStyle;
//   labelStyle.x = style.x + 32 + 8;
//   labelStyle.fontFamily = FontFamily::PARAGRAPH;
//   labelStyle.fontSize = sdl2w::TEXT_SIZE_16;
//   labelStyle.fontColor = Colors::Black;
//   labelStyle.textAlign = TextAlign::LEFT_TOP;
//   label->setStyle(labelStyle);
//   // Set label text
//   TextLineProps labelProps;
//   TextBlock labelBlock;
//   labelBlock.text = itemTemplate.label.empty() ? itemTemplate.name :
//   itemTemplate.label; labelProps.textBlocks.push_back(labelBlock);
//   label->setProps(labelProps);
//   labelStyle.y = style.y + (listItemHeight - label->getDims().second) / 2;
//   label->setStyle(labelStyle);
//   children.push_back(std::move(label));

//   // Create context button
//   auto contextBtn = std::make_unique<ButtonModal>(window, this);
//   contextBtn->setId("contextBtn");
//   BaseStyle buttonStyle = contextBtn->getStyle();
//   buttonStyle.x = style.x + style.width - 32; // Right side
//   buttonStyle.y = style.y + (listItemHeight - 32) / 2;
//   buttonStyle.width = 32;
//   buttonStyle.height = 32;
//   buttonStyle.scale = 1.0f;
//   contextBtn->setStyle(buttonStyle);

//   ButtonModalProps buttonProps;
//   buttonProps.text = "*"; // Context menu indicator
//   buttonProps.isSelected = false;
//   contextBtn->setProps(buttonProps);
//   contextBtn->addEventObserver(new ListInventoryItemContextButtonObserver(
//       window, itemTemplate.name, props.itemId));
//   children.push_back(std::move(contextBtn));
// }

// void ListInventoryItem::render(int dt) {
//   auto& draw = window->getDraw();
//   draw.drawRect(style.x, style.y, style.width, style.height, bgColor);
//   UiElement::render(dt);
// }

} // namespace ui
