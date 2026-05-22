#include "PageInventory.h"
#include "lib/sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/lists/ListInventory.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"

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

void PageInventory::populateInventoryProps(
    std::vector<ListInventoryPropsItem>& listProps) {
  if (!getStateManager() || !props.characterPlayer || !getDatabase()) {
    return;
  }
  auto& database = *getDatabase();
  auto& inventory = props.characterPlayer->inventory;

  for (const auto& item : inventory) {
    auto& itemTemplate = database.getItemTemplate(item.itemName);
    listProps.push_back({.itemName = itemTemplate.name,
                         .itemLabel = itemTemplate.label,
                         .itemSprite = itemTemplate.iconSpriteName});
  }
}

void PageInventory::build() {
  children.clear();

  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  auto& modalStyle = modal->getStyle();
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setProps(ModalStandardProps{});
  addChild(modal);

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLoc();
  int unscaledContentW = contentW / style.scale;
  int unscaledContentH = contentH / style.scale;

  // Create title element
  auto title = new TextLine(window, modal);
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = TRANSLATE("Inventory");
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  // Create SectionScrollable for content area
  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  BaseStyle scrollableStyle;
  scrollableStyle.x = contentX;
  scrollableStyle.y = contentY;
  scrollableStyle.width = unscaledContentW;
  scrollableStyle.height = unscaledContentH;
  scrollableStyle.scale = style.scale;
  scrollableSection->setStyle(scrollableStyle);
  scrollableSection->setProps(SectionScrollableProps{.scrollBarWidth = 40});
  addChild(scrollableSection);

  // Create ListInventory inside the scrollable section
  auto listInventory = new ListInventory(window, scrollableSection);
  listInventory->setId("listInventory");
  auto& listStyle = listInventory->getStyle();
  listStyle.x = 0; // Positioned relative to scrollable section
  listStyle.y = 0;
  listStyle.width = static_cast<float>(contentW) /
                        style.scale - // contentW and contentH are already scaled
                    scrollableSection->getProps().scrollBarWidth -
                    8;
  listStyle.scale = style.scale;

  ListInventoryProps listProps;
  populateInventoryProps(listProps.items);
  listInventory->setProps(listProps);
  scrollableSection->addChild(listInventory);
  scrollableSection->build();
}

void PageInventory::render(int dt) { UiElement::render(dt); }

} // namespace ui
