#include "MinipageSpellCast.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include "ui/components/lists/ListMagicSpells.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/helpers/modalLayoutFit.h"
#include "ui/layouts/ModalSmall.h"
#include "ui/observers/ObserverRemoveLayer.hpp"

namespace ui {

MinipageSpellCast::MinipageSpellCast(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void MinipageSpellCast::setProps(const MinipageSpellCastProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

MinipageSpellCastProps& MinipageSpellCast::getProps() { return props; }

const MinipageSpellCastProps& MinipageSpellCast::getProps() const { return props; }

const std::pair<int, int> MinipageSpellCast::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void MinipageSpellCast::build() {
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
      .iconSprite = "ui_action_buttons_half_16",
      .enableCloseButton = false,
  });
  syncHostStyleToCappedCentered(style, ModalSizeClass::Small);
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
  (void)scrollableContentH;

  auto listSpells = new ListMagicSpells(window, scrollableSection);
  listSpells->setId("listSpells");
  listSpells->setPos(4 * style.scale, 4 * style.scale);
  listSpells->setScale(style.scale);

  ListMagicSpellsProps listProps;
  listProps.casterId = props.casterId;
  listProps.width = scrollableContentW / style.scale - 8;
  listProps.enableSpellInfoOnClick = false;
  listProps.enableSpellCastOnClick = true;
  for (const auto& spell : props.spells) {
    listProps.spells.pushBack(ListMagicSpellsPropsSpell{
        .id = spell.id,
        .label = spell.label,
        .iconSprite = spell.iconSprite,
        .requiredRuneSprites = spell.requiredRuneSprites,
    });
  }
  listSpells->setProps(listProps);

  scrollableSection->addChild(listSpells);
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
}

void MinipageSpellCast::render(int dt) { UiElement::render(dt); }

} // namespace ui
