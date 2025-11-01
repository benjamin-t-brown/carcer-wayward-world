#include "PageInventory.h"
#include "model/Character.h"
#include "ui/colors.h"
#include "ui/components/BorderModalStandard.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/lists/ListInventory.h"

namespace ui {

PageInventory::PageInventory(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageInventory::setProps(const PageInventoryProps& _props) {
  props = _props;
  build();
}

PageInventoryProps& PageInventory::getProps() { return props; }

const PageInventoryProps& PageInventory::getProps() const { return props; }

const std::pair<int, int> PageInventory::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageInventory::build() {
  children.clear();

  if (props.characterPlayerId.empty()) {
    return;
  }

  // Look up the character player by ID
  auto* characterPlayer =
      getStateManager()->getState().player.findPartyMember(props.characterPlayerId);
  if (characterPlayer == nullptr) {
    LOG(ERROR) << "PageInventory::build: character player not found" << LOG_ENDL;
    return;
  }

  // Create ModalStandard layout
  auto modal = std::make_unique<ModalStandard>(window, this);
  modal->setId("modal");
  BaseStyle modalStyle;
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setStyle(modalStyle);

  ModalStandardProps modalProps;
  modal->setProps(modalProps);

  auto contentDims = modal->getContentDims();

  // // Get the border element to access location methods
  // auto borderElement = dynamic_cast<BorderModalStandard*>(modal->getChildById("border"));
  // if (borderElement == nullptr) {
  //   return;
  // }

  // Create title element
  auto title = std::make_unique<TextLine>(window, modal.get());
  BaseStyle titleStyle;
  titleStyle.fontFamily = FontFamily::H2;
  titleStyle.fontSize = sdl2w::TEXT_SIZE_20;
  titleStyle.fontColor = Colors::White;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  title->setStyle(titleStyle);
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = "Inventory";
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(std::move(title));

  // Create subtitle element
  auto subtitle = std::make_unique<TextLine>(window, modal.get());
  BaseStyle subtitleStyle;
  subtitleStyle.fontFamily = FontFamily::PARAGRAPH;
  subtitleStyle.fontSize = sdl2w::TEXT_SIZE_16;
  subtitleStyle.fontColor = Colors::White;
  subtitleStyle.textAlign = TextAlign::LEFT_TOP;
  subtitle->setStyle(subtitleStyle);
  TextLineProps subtitleProps;
  TextBlock subtitleBlock;
  subtitleBlock.text = "Subtitle";
  subtitleProps.textBlocks.push_back(subtitleBlock);
  subtitle->setProps(subtitleProps);
  modal->setSubtitleElement(std::move(subtitle));

  // Get content location and dimensions from border
  // auto contentLocation = borderElement->getContentLocation();
  // auto contentDims = borderElement->getContentDims();

  // Create SectionScrollable for content area
  auto scrollableSection = std::make_unique<SectionScrollable>(window, modal.get());
  scrollableSection->setId("scrollableSection");
  BaseStyle scrollableStyle;
  scrollableStyle.width = contentDims.first;
  scrollableStyle.height = contentDims.second;
  scrollableStyle.scale = 1;
  scrollableSection->setStyle(scrollableStyle);

  SectionScrollableProps scrollableProps;
  scrollableSection->setProps(scrollableProps);

  // Create ListInventory inside the scrollable section
  auto listInventory = std::make_unique<ListInventory>(window, scrollableSection.get());
  listInventory->setId("listInventory");
  BaseStyle listStyle;
  listStyle.x = 0; // Positioned relative to scrollable section
  listStyle.y = 4;
  listStyle.width = contentDims.first - scrollableProps.scrollBarWidth - 8;
  listStyle.scale = 1;
  listInventory->setStyle(listStyle);

  ListInventoryProps listProps;
  listProps.character = characterPlayer;
  listInventory->setProps(listProps);

  // Add ListInventory to SectionScrollable
  scrollableSection->addChild(std::move(listInventory));

  // Set SectionScrollable as content element of ModalStandard
  modal->setContentElement(std::move(scrollableSection));

  // Add modal to children
  children.push_back(std::move(modal));
}

void PageInventory::render(int dt) {
  UiElement::render(dt);
}

} // namespace ui

