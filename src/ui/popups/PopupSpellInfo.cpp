#include "PopupSpellInfo.h"
#include "sdl2w/L10n.h"
#include "ui/colors.hpp"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/observers/ObserverRemoveLayer.hpp"

namespace ui {

PopupSpellInfo::PopupSpellInfo(sdl2w::Window* _window,
                               UiElement* _parent,
                               PopupOrientation _orientation)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  props.orientation = _orientation;
  build();
}

void PopupSpellInfo::setProps(const PopupSpellInfoProps& _props) {
  props = _props;
  build();
}

PopupSpellInfoProps& PopupSpellInfo::getProps() { return props; }

const PopupSpellInfoProps& PopupSpellInfo::getProps() const { return props; }

void PopupSpellInfo::build() {
  children.clear();

  if (props.orientation == PopupOrientation::NARROW) {
    style.width = 300;
    style.height = 320;
  } else {
    style.width = 400;
    style.height = 260;
  }

  const int padding = 8;
  const int closeButtonSize = 32;
  const int iconBgSize = 32;
  const int spellIconSize = 24;
  const int runeIconSize = 24;
  const int vertSpacer = 8;

  auto [scaledWidth, scaledHeight] = getDims();

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  addChild(border);

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  closeButton->setPos(
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale,
      style.y + padding * style.scale);
  closeButton->setScale(style.scale);
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(
      new ObserverRemoveLayer(state::LayerId::SpellInfo));
  addChild(closeButton);

  auto spriteBgQuad = new Quad(window, this);
  spriteBgQuad->setId("spriteBg");
  spriteBgQuad->setPos(style.x + padding * style.scale, style.y + padding * style.scale);
  spriteBgQuad->setScale(style.scale);
  spriteBgQuad->setProps(QuadProps{
      .width = iconBgSize,
      .height = iconBgSize,
      .bgColor = Colors::LightGrey,
  });
  addChild(spriteBgQuad);

  if (!props.spriteName.empty()) {
    const int iconOffset = (iconBgSize - spellIconSize) / 2;
    auto sprite = new SpriteElement(window, spriteBgQuad);
    sprite->setId("icon");
    sprite->setPos(iconOffset, iconOffset);
    sprite->setScale(style.scale);
    sprite->setProps(SpriteElementProps{
        .width = spellIconSize,
        .height = spellIconSize,
        .spriteName = props.spriteName,
    });
    spriteBgQuad->addChild(sprite);
  }

  auto label = new TextLine(window, this);
  label->setId("label");
  TextFontProps labelFont;
  setBaseFontConfig(labelFont, BaseFontConfig::MODAL_TITLE);
  label->setPos(
      style.x + padding * style.scale + iconBgSize * style.scale + padding * style.scale,
      style.y + padding * style.scale + (iconBgSize * style.scale) / 2);
  label->setScale(1.f);
  label->setProps(TextLineProps{
      .textBlocks = {{.text = props.label}},
      .fontFamily = labelFont.fontFamily,
      .fontSize = labelFont.fontSize,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  addChild(label);

  TextFontProps bodyFont;
  setBaseFontConfig(bodyFont, BaseFontConfig::MODAL_TEXT);

  int contentY = style.y + padding * style.scale + iconBgSize * style.scale +
                 vertSpacer * style.scale;
  const int contentX = style.x + padding * style.scale;
  const int contentWidth =
      static_cast<int>((scaledWidth - padding * 2 * style.scale) / style.scale);

  auto manaLine = new TextLine(window, this);
  manaLine->setId("manaCost");
  manaLine->setPos(contentX, contentY);
  manaLine->setScale(1.f);
  manaLine->setProps(TextLineProps{
      .textBlocks =
          {{.text = TRANSLATE("Mana: ") + bmin::toString(props.manaCost)}},
      .fontFamily = bodyFont.fontFamily,
      .fontSize = bodyFont.fontSize,
      .fontColor = Colors::DarkBlue,
      .textAlign = TextAlign::LEFT_TOP,
  });
  addChild(manaLine);
  contentY += manaLine->getDims().second + vertSpacer * style.scale;

  auto runesHeader = new TextLine(window, this);
  runesHeader->setId("runesHeader");
  runesHeader->setPos(contentX, contentY);
  runesHeader->setScale(1.f);
  runesHeader->setProps(TextLineProps{
      .textBlocks = {{.text = TRANSLATE("Required runes:")}},
      .fontFamily = bodyFont.fontFamily,
      .fontSize = bodyFont.fontSize,
      .fontColor = Colors::DarkGrey,
      .textAlign = TextAlign::LEFT_TOP,
  });
  addChild(runesHeader);
  contentY += runesHeader->getDims().second + 4 * style.scale;

  if (props.requiredRunes.empty()) {
    auto noneLine = new TextLine(window, this);
    noneLine->setId("runesNone");
    noneLine->setPos(contentX, contentY);
    noneLine->setScale(1.f);
    noneLine->setProps(TextLineProps{
        .textBlocks = {{.text = TRANSLATE("None")}},
        .fontFamily = bodyFont.fontFamily,
        .fontSize = bodyFont.fontSize,
        .fontColor = Colors::DarkGrey,
        .textAlign = TextAlign::LEFT_TOP,
    });
    addChild(noneLine);
    contentY += noneLine->getDims().second + vertSpacer * style.scale;
  } else {
    int runeX = contentX;
    const int runeGap = static_cast<int>(4 * style.scale);
    const int groupGap = static_cast<int>(10 * style.scale);
    for (size_t i = 0; i < props.requiredRunes.size(); ++i) {
      const auto& req = props.requiredRunes[i];
      const int count = req.count > 0 ? req.count : 1;
      for (int n = 0; n < count; ++n) {
        if (!req.iconSprite.empty()) {
          auto runeIcon = new SpriteElement(window, this);
          runeIcon->setId("rune_" + bmin::toString(static_cast<int>(i)) + "_" +
                          bmin::toString(n));
          runeIcon->setPos(runeX, contentY);
          runeIcon->setScale(style.scale);
          runeIcon->setProps(SpriteElementProps{
              .width = runeIconSize,
              .height = runeIconSize,
              .spriteName = req.iconSprite,
          });
          addChild(runeIcon);
        }
        runeX += static_cast<int>(runeIconSize * style.scale) + runeGap;
      }
      runeX += groupGap - runeGap;
    }
    contentY += static_cast<int>(runeIconSize * style.scale) + vertSpacer * style.scale;
  }

  auto description = new TextParagraph(window, this);
  description->setId("description");
  description->setPos(contentX, contentY);
  description->setScale(1.f);
  TextParagraphProps descProps;
  descProps.width = contentWidth;
  descProps.fontFamily = bodyFont.fontFamily;
  descProps.fontSize = bodyFont.fontSize;
  descProps.fontColor = Colors::Black;
  descProps.textAlign = TextAlign::LEFT_TOP;
  descProps.lineSpacing = 0;
  descProps.textBlocks.pushBack({.text = props.description});
  description->setProps(descProps);
  addChild(description);
}

void PopupSpellInfo::render(int dt) { UiElement::render(dt); }

} // namespace ui
