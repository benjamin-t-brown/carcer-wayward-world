#include "MinipagePickUp.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/PartyMemberIconSelector.h"
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
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto modal = new ModalSmall(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalSmallProps{
      .width = style.width,
      .height = style.height,
      .enableCloseButton = false,
  });
  addChild(modal);

  auto [contentW, contentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();
  int unscaledContentW = static_cast<int>(contentW / style.scale);
  int unscaledContentH = static_cast<int>(contentH / style.scale);

  auto title = new TextLine(window, modal);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = Colors::Black;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = props.titleText;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  auto scrollableSection = new SectionScrollable(window, modal);
  scrollableSection->setId("scrollableSection");
  scrollableSection->setPos(contentX, contentY);
  scrollableSection->setScale(style.scale);
  scrollableSection->setProps(SectionScrollableProps{
      .width = unscaledContentW,
      .height = unscaledContentH,
  });
  auto [scrollableContentW, scrollableContentH] = scrollableSection->getContentDims();

  auto listPickUp = new ListPickUp(window, scrollableSection);
  listPickUp->setId("listPickUp");
  listPickUp->setPos(4 * style.scale, 4 * style.scale);
  listPickUp->setScale(style.scale);

  ListPickUpProps listProps;
  listProps.width = scrollableContentW / style.scale - 8;
  if (getDatabase()) {
    for (const auto& item : props.nearbyItems) {
      const auto& itemTemplate = getDatabase()->getItemTemplate(bmin::toStringView(item.itemTemplateName));
      listProps.items.pushBack({
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
  buttonGroup->setPos(buttonsX, buttonsY);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(ButtonGroupProps{
      .width = static_cast<int>(buttonsW / style.scale),
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
  TextFontProps statusFont;
  setBaseFontConfig(statusFont, BaseFontConfig::MODAL_TEXT);
  statusText->setPos(buttonsX + static_cast<int>(8 * style.scale),
                     buttonsY + buttonsH / 2);
  statusText->setScale(style.scale);
  statusText->setProps({
      .textBlocks =
          {
              {
                  .text = props.statusText,
              },
          },
      .fontFamily = statusFont.fontFamily,
      .fontSize = statusFont.fontSize,
      .fontColor = Colors::DarkGrey,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  modal->addChild(statusText);

  if (!props.partyMemberSprites.empty()) {
    auto partySelector = new PartyMemberIconSelector(window, this);
    partySelector->setId("partyMemberSelector");
    partySelector->setScale(style.scale);
    partySelector->setProps(PartyMemberIconSelectorProps{
        .members = props.partyMemberSprites,
        .selectedIndex = props.partyMemberIndex,
        .target = PartyMemberIconSelectorTarget::PICKUP,
    });
    auto [selectorW, selectorH] = partySelector->getDims();
    partySelector->setPos(contentX + contentW - selectorW - static_cast<int>(4 * style.scale),
                          style.y + static_cast<int>(8 * style.scale));
    partySelector->build();

    auto weightText = new TextLine(window, this);
    weightText->setId("weightText");
    TextFontProps weightFont;
    setBaseFontConfig(weightFont, BaseFontConfig::MODAL_TEXT);
    auto [selectorX, selectorY] = partySelector->getPos();
    weightText->setPos(selectorX + selectorW / 2,
                       selectorY + selectorH + static_cast<int>(16 * style.scale));
    weightText->setScale(1.f);
    weightText->setProps({
        .textBlocks =
            {
                {
                    .text = props.weightText,
                },
            },
        .fontFamily = weightFont.fontFamily,
        .fontSize = weightFont.fontSize,
        .fontColor = Colors::DarkGrey,
        .textAlign = TextAlign::CENTER,
    });

    addChild(partySelector);
    addChild(weightText);
  }
}

void MinipagePickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui
