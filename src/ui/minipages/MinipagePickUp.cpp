#include "MinipagePickUp.h"
#include "lib/sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/PartyMemberSwitcher.h"
#include "ui/components/lists/ListPickUp.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/layouts/ModalSmall.h"
#include "ui/observers/ObserverRemoveLayer.hpp"

namespace ui {

MinipagePickUp::MinipagePickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Minipage doesn't need special initialization.
}

void MinipagePickUp::setProps(const MinipagePickUpProps& _props) {
  props = _props;
  build();
}

MinipagePickUpProps& MinipagePickUp::getProps() { return props; }

const MinipagePickUpProps& MinipagePickUp::getProps() const { return props; }

const std::pair<int, int> MinipagePickUp::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipagePickUp::build() {
  children.clear();

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  auto& modalStyle = modal->getStyle();
  modalStyle.x = style.x;
  modalStyle.y = style.y;
  modalStyle.width = style.width;
  modalStyle.height = style.height;
  modalStyle.scale = style.scale;
  modal->setProps(ModalSmallProps{
      .enableCloseButton = false,
  });
  addChild(modal);

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = static_cast<int>(contentW / style.scale);
  int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto title = new TextLine(window, modal);
  auto& titleStyle = title->getStyle();
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.fontColor = Colors::Black;
  titleStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps titleProps;
  TextBlock titleBlock;
  titleBlock.text = props.titleText;
  titleProps.textBlocks.push_back(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  auto& scrollableStyle = scrollableSection->getStyle();
  scrollableStyle.width = unscaledContentW;
  scrollableStyle.height = unscaledContentH;
  scrollableStyle.scale = style.scale;
  scrollableStyle.x = contentX;
  scrollableStyle.y = contentY;
  scrollableSection->setProps(SectionScrollableProps{});
  auto [scrollableContentW, scrollableContentH] = scrollableSection->getContentDims();

  auto listPickUp = new ListPickUp(window, scrollableSection);
  listPickUp->setId("listPickUp");
  auto& listStyle = listPickUp->getStyle();
  listStyle.x = 4 * style.scale;
  listStyle.y = 4 * style.scale;
  listStyle.width = scrollableContentW / style.scale - 8;
  listStyle.scale = style.scale;

  ListPickUpProps listProps;
  if (getDatabase()) {
    for (const auto& item : props.nearbyItems) {
      const auto& itemTemplate = getDatabase()->getItemTemplate(item.itemName);
      listProps.items.push_back({
          .item = item,
          .itemLabel =
              itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label,
          .weight = itemTemplate.weight,
          .itemSprite = itemTemplate.iconSpriteName,
      });
    }
  }
  listPickUp->setProps(listProps);

  scrollableSection->addChild(listPickUp);
  scrollableSection->build();
  modal->addChild(scrollableSection);

  auto [buttonsW, buttonsH] = modal->getButtonsDims();
  auto [buttonsX, buttonsY] = modal->getButtonsLocation();
  const int buttonPadding = static_cast<int>(2);
  const int buttonWidth = 120;
  const int buttonHeight = ModalSmall::BUTTONS_AREA_HEIGHT;

  auto buttonGroup = new ButtonGroup(window, modal);
  buttonGroup->setId("buttonGroup");
  auto& buttonGroupStyle = buttonGroup->getStyle();
  buttonGroupStyle.x = buttonsX;
  buttonGroupStyle.y = buttonsY;
  buttonGroupStyle.width = buttonsW / style.scale;
  buttonGroupStyle.height = buttonsH / style.scale;
  buttonGroupStyle.scale = style.scale;
  buttonGroup->setProps(ButtonGroupProps{
      .alignment = ButtonGroupAlignment::RIGHT,
      .buttonWidth = buttonWidth,
      .buttonHeight = buttonHeight - 2 * buttonPadding,
      .padding = buttonPadding,
      .buttons = {{//
                   .label = TRANSLATE("Done"),
                   .type = ButtonGroupButtonType::MODAL}},
  });
  if (!props.doneButtonRemoveLayerId.empty()) {
    buttonGroup->addObserverToButtonAtIndex(
        0, new ObserverRemoveLayer(props.doneButtonRemoveLayerId));
  }
  modal->addChild(buttonGroup);

  auto statusText = new TextLine(window, modal);
  statusText->setId("StatusText");
  auto& statusTextStyle = statusText->getStyle();
  setBaseFontConfig(statusTextStyle, BaseFontConfig::MODAL_TEXT);
  statusTextStyle.fontColor = Colors::DarkGrey;
  statusTextStyle.textAlign = TextAlign::LEFT_CENTER;
  statusTextStyle.scale = style.scale;
  statusTextStyle.x = buttonsX + static_cast<int>(8 * style.scale);
  statusTextStyle.y = buttonsY + buttonsH / 2;
  statusText->setProps({
      .textBlocks =
          {
              {
                  .text = props.statusText,
              },
          },
  });
  modal->addChild(statusText);

  auto partyMemberSwitcher = new PartyMemberSwitcher(window, this);
  partyMemberSwitcher->setId("partyMemberSwitcher");
  auto& switcherStyle = partyMemberSwitcher->getStyle();
  switcherStyle.scale = style.scale;
  partyMemberSwitcher->setProps(PartyMemberSwitcherProps{
      .spriteName = props.partyMemberSpriteName,
      .partyMemberIndex = props.partyMemberIndex,
  });
  auto [switcherW, switcherH] = partyMemberSwitcher->getDims();
  switcherStyle.x = contentX + contentW - switcherW - static_cast<int>(16 * style.scale);
  switcherStyle.y = style.y + static_cast<int>(8 * style.scale);
  partyMemberSwitcher->build();

  auto weightText = new TextLine(window, this);
  weightText->setId("weightText");
  auto& weightTextStyle = weightText->getStyle();
  setBaseFontConfig(weightTextStyle, BaseFontConfig::MODAL_TEXT);
  weightTextStyle.fontColor = Colors::DarkGrey;
  weightTextStyle.textAlign = TextAlign::CENTER;
  weightTextStyle.scale = 1.;
  weightTextStyle.x = switcherStyle.x + switcherW / 2;
  weightTextStyle.y = switcherStyle.y + switcherH + static_cast<int>(16 * style.scale);
  weightText->setProps({
      .textBlocks =
          {
              {
                  .text = props.weightText,
              },
          },
  });

  addChild(partyMemberSwitcher);
  addChild(weightText);
}

void MinipagePickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui
