#include "PopupPickupItem.h"
#include "lib/sdl2w/L10n.h"
#include "lib/sdl2w/Logger.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/colors.h"
#include "ui/elements/ButtonClose.h"
#include "ui/elements/SectionDropShadow.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"

namespace ui {

class PopupPickupItemCloseButtonObserver : public UiEventObserver,
                                           public state::StateManagerInterface {

  layers::Layer* layer;

public:
  PopupPickupItemCloseButtonObserver(layers::Layer* _layer) : layer(_layer) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "PopupPickupItemCloseButtonObserver::onClick" << LOG_ENDL;
    getStateManager()->enqueueAction(
        getStateManager()->getState(), new state::actions::UiRemoveLayer(layer, true), 0);
  }
};

PopupPickupItem::PopupPickupItem(sdl2w::Window* _window, layers::Layer* _layer)
    : UiElement(_window, nullptr) {
  this->layer = _layer;
  shouldPropagateEventsToChildren = true;
}

void PopupPickupItem::setProps(const PopupPickupItemProps& _props) {
  props = _props;
  build();
}

PopupPickupItemProps& PopupPickupItem::getProps() { return props; }

const PopupPickupItemProps& PopupPickupItem::getProps() const { return props; }

const std::pair<int, int> PopupPickupItem::getDims() const {
  if (children.empty()) {
    return {style.width, 0};
  }
  return children[0]->getDims();
}

void PopupPickupItem::build() {
  children.clear();

  if (props.itemName.empty()) {
    return;
  }

  // Get the item template from database using itemName
  const auto& itemTemplate = getDatabase()->getItemTemplate(props.itemName);

  // Create the SectionDropShadow as the container
  auto section = std::make_unique<SectionDropShadow>(window, this);

  // Set default props for the section
  SectionDropShadowProps sectionProps = section->getProps();
  sectionProps.backgroundColor = Colors::White;
  sectionProps.shadowColor = Colors::Black;
  sectionProps.shadowOffsetX = -8;
  sectionProps.shadowOffsetY = 8;
  sectionProps.borderSize = 2;
  section->setProps(sectionProps);

  // Padding constants
  const int padding = 4;
  const int iconSize = 32;
  const int spacing = 12;
  const int textPaddingLeft = 16;

  // Add ButtonClose in top right
  auto closeButton = std::make_unique<ButtonClose>(window, section.get());
  closeButton->setId("closeButton");
  BaseStyle closeStyle;
  closeStyle.x = style.width - 32 - padding;
  closeStyle.y = padding;
  closeStyle.width = 32;
  closeStyle.height = 32;
  closeStyle.scale = style.scale;
  closeButton->setStyle(closeStyle);
  ButtonCloseProps closeProps;
  closeProps.closeType = CloseType::POPUP;
  closeButton->setProps(closeProps);
  closeButton->addEventObserver(
      std::unique_ptr<UiEventObserver>(new PopupPickupItemCloseButtonObserver(layer)));
  section->addChild(std::move(closeButton));

  int currentY = padding;

  // Add item icon
  auto icon = std::make_unique<SpriteElement>(window, section.get());
  icon->setId("icon");
  BaseStyle iconStyle;
  iconStyle.x = padding + 4;
  iconStyle.y = currentY + 4;
  iconStyle.width = iconSize;
  iconStyle.height = iconSize;
  iconStyle.scale = 2;
  icon->setStyle(iconStyle);
  icon->setSprite(itemTemplate.iconSpriteName);
  section->addChild(std::move(icon));

  // Add item label next to icon
  auto label = std::make_unique<TextLine>(window, section.get());
  label->setId("label");
  BaseStyle labelStyle;
  labelStyle.x = textPaddingLeft + iconSize + spacing;
  labelStyle.y = currentY + (iconSize - 16) / 2; // Vertically center with icon
  labelStyle.width = style.width - padding - iconSize - spacing - padding - 32 -
                     spacing; // Leave space for close button
  labelStyle.fontFamily = FontFamily::H2;
  labelStyle.fontSize = sdl2w::TEXT_SIZE_20;
  labelStyle.fontColor = Colors::Black;
  labelStyle.textAlign = TextAlign::LEFT_TOP;
  label->setStyle(labelStyle);
  TextLineProps labelProps;
  TextBlock labelBlock;
  labelBlock.text = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  labelProps.textBlocks.push_back(labelBlock);
  label->setProps(labelProps);
  section->addChild(std::move(label));

  // Update currentY for next element
  currentY += iconSize + spacing;

  // Add description paragraph
  if (!itemTemplate.description.empty()) {
    auto description = std::make_unique<TextParagraph>(window, section.get());
    description->setId("description");
    BaseStyle descStyle;
    descStyle.x = textPaddingLeft;
    descStyle.y = currentY;
    descStyle.width = style.width - padding - textPaddingLeft - padding;
    descStyle.fontFamily = FontFamily::PARAGRAPH;
    descStyle.fontSize = sdl2w::TEXT_SIZE_16;
    descStyle.fontColor = Colors::Black;
    descStyle.textAlign = TextAlign::LEFT_TOP;
    descStyle.lineSpacing = 4;
    description->setStyle(descStyle);
    TextParagraphProps descProps;
    TextBlock descBlock;
    descBlock.text = itemTemplate.description;
    descProps.textBlocks.push_back(descBlock);
    description->setProps(descProps);

    // Get height before moving
    auto descHeight = description->getDims().second;
    section->addChild(std::move(description));

    // Update currentY based on description height
    currentY += descHeight + spacing;
  }

  // Add weight and value
  // Weight
  auto weightLine = std::make_unique<TextLine>(window, section.get());
  weightLine->setId("weight");
  BaseStyle weightStyle;
  weightStyle.x = textPaddingLeft;
  weightStyle.y = currentY;
  weightStyle.width = style.width - 2 * padding;
  weightStyle.fontFamily = FontFamily::PARAGRAPH;
  weightStyle.fontSize = sdl2w::TEXT_SIZE_16;
  weightStyle.fontColor = Colors::Black;
  weightStyle.textAlign = TextAlign::LEFT_TOP;
  weightLine->setStyle(weightStyle);
  TextLineProps weightProps;
  TextBlock weightBlock;
  weightBlock.text = TRANSLATE("Weight: ") + std::to_string(itemTemplate.weight) + " lbs";
  weightProps.textBlocks.push_back(weightBlock);
  weightLine->setProps(weightProps);
  auto weightLineHeight = weightLine->getDims().second;
  section->addChild(std::move(weightLine));

  // Update currentY for next stat line
  currentY += weightLineHeight + spacing;

  // Value
  auto valueLine = std::make_unique<TextLine>(window, section.get());
  valueLine->setId("value");
  BaseStyle valueStyle;
  valueStyle.x = textPaddingLeft;
  valueStyle.y = currentY;
  valueStyle.width = style.width - 2 * padding;
  valueStyle.fontFamily = FontFamily::PARAGRAPH;
  valueStyle.fontSize = sdl2w::TEXT_SIZE_16;
  valueStyle.fontColor = Colors::Black;
  valueStyle.textAlign = TextAlign::LEFT_TOP;
  valueLine->setStyle(valueStyle);
  TextLineProps valueProps;
  TextBlock valueBlock;
  valueBlock.text = TRANSLATE("Value: ") + std::to_string(itemTemplate.value) + " gp";
  valueProps.textBlocks.push_back(valueBlock);
  valueLine->setProps(valueProps);
  auto valueLineHeight = valueLine->getDims().second;
  section->addChild(std::move(valueLine));
  currentY += valueLineHeight + spacing;

  getStyle().height = currentY;

  // set section style, then push it to children
  BaseStyle sectionStyle = section->getStyle();
  sectionStyle.x = style.x;
  sectionStyle.y = style.y;
  sectionStyle.width = style.width;
  sectionStyle.height = getStyle().height;
  sectionStyle.scale = 1;
  section->setStyle(sectionStyle);

  children.push_back(std::move(section));
}

void PopupPickupItem::render(int dt) { UiElement::render(dt); }

} // namespace ui

