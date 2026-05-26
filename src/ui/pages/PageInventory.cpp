#include "PageInventory.h"
#include "layers/ui/LayerInventory.h"
#include "lib/sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/lists/ListInventory.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include "ui/observers/ObserverUpdateCurrentPartyMember.hpp"
#include <algorithm>

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
  if (!getStateManager() || !getDatabase()) {
    return;
  }
  auto& database = *getDatabase();

  for (const auto& item : props.inventory) {
    auto& itemTemplate = database.getItemTemplate(item.itemName);
    listProps.push_back(
        {.itemId = item.id,
         .itemName = item.itemName,
         .itemLabel = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label,
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
  ModalStandardProps modalProps;
  if (!props.characterPlayerSprite.empty()) {
    modalProps.iconSprite = props.characterPlayerSprite;
  }
  modal->setProps(modalProps);
  addChild(modal);

  if (!props.characterPlayerSprite.empty()) {
    if (auto* icon = modal->getChildById("headerIcon")) {
      auto iconBg = std::make_unique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setStyle(icon->getStyle());
      iconBg->setProps(QuadProps{.bgColor = {255, 255, 255, 50}});

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(
          modalChildren.begin(),
          modalChildren.end(),
          [](const std::unique_ptr<UiElement>& child) {
            return child->getId() == "headerIcon";
          });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, std::move(iconBg));
      }
    }
  }

  auto closeButton = modal->getCloseButtonElement();
  if (closeButton) {
    closeButton->addEventObserver(
        new ObserverRemoveLayer(layers::LayerInventory::LAYER_ID));
  }

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = contentW / style.scale;
  int unscaledContentH = contentH / style.scale;

  // Create title element
  auto title = new TextLine(window, modal);
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text =
      std::string(TRANSLATE("Inventory")) + " - " + props.characterPlayerLabel;
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto [subtitleX, subtitleY] = modal->getSubTitleLocation();
  auto [subtitleW, subtitleH] = modal->getSubTitleDims();
  (void)subtitleH;

  const int navButtonSize = ButtonClose::closeButtonSize;
  const int navSpacing = static_cast<int>(4 * style.scale);
  const int scaledNavButtonSize = static_cast<int>(navButtonSize * style.scale);
  const int navButtonY = subtitleY - scaledNavButtonSize / 2;

  auto prevPartyButton = new ButtonModal(window, modal);
  prevPartyButton->setId("prevPartyMember");
  auto& prevStyle = prevPartyButton->getStyle();
  prevStyle.x = subtitleX;
  prevStyle.y = navButtonY;
  prevStyle.width = navButtonSize;
  prevStyle.height = navButtonSize;
  prevStyle.scale = style.scale;
  prevPartyButton->setProps(ButtonModalProps{.text = "<"});
  prevPartyButton->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, -1));
  modal->addChild(prevPartyButton);

  auto nextPartyButton = new ButtonModal(window, modal);
  nextPartyButton->setId("nextPartyMember");
  auto& nextStyle = nextPartyButton->getStyle();
  nextStyle.x = subtitleX + scaledNavButtonSize + navSpacing;
  nextStyle.y = navButtonY;
  nextStyle.width = navButtonSize;
  nextStyle.height = navButtonSize;
  nextStyle.scale = style.scale;
  nextPartyButton->setProps(ButtonModalProps{.text = ">"});
  nextPartyButton->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, 1));
  modal->addChild(nextPartyButton);

  auto goldSubtitle = new TextLine(window, modal);
  goldSubtitle->setId("subtitleGold");
  auto& goldStyle = goldSubtitle->getStyle();
  setBaseFontConfig(goldStyle, BaseFontConfig::MODAL_TEXT);
  goldStyle.fontColor = Colors::DarkGrey;
  goldStyle.textAlign = TextAlign::LEFT_CENTER;
  goldStyle.scale = 1.f;
  goldStyle.y = subtitleY;
  goldSubtitle->setProps(TextLineProps{
      .textBlocks = {{
          .text = std::to_string(props.gold) + std::string(TRANSLATE(" gp")),
          .fontColor = Colors::Blue,
      }},
  });
  goldStyle.x = subtitleX + subtitleW - goldSubtitle->getDims().first - 16 * style.scale;
  goldSubtitle->build();

  auto weightSubtitle = new TextLine(window, modal);
  weightSubtitle->setId("subtitleWeight");
  auto& weightStyle = weightSubtitle->getStyle();
  setBaseFontConfig(weightStyle, BaseFontConfig::MODAL_TEXT);
  weightStyle.fontColor = Colors::DarkGrey;
  weightStyle.textAlign = TextAlign::LEFT_CENTER;
  weightStyle.scale = 1.f;
  weightStyle.y = subtitleY;
  weightSubtitle->setProps(TextLineProps{
      .textBlocks = {{
          .text = std::string(TRANSLATE("Carrying")) + " " +
                  std::to_string(props.weightCarrying) + "/" +
                  std::to_string(props.weightCapacity),
      }},
  });
  weightStyle.x =
      goldStyle.x - navSpacing - weightSubtitle->getDims().first - 16 * style.scale;
  weightSubtitle->build();

  modal->addChild(weightSubtitle);
  modal->addChild(goldSubtitle);

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
