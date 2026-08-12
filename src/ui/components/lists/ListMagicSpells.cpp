#include "ListMagicSpells.h"
#include "bmin/StringInterop.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/VerticalList.h"
#include "ui/observers/ObserverShowLayerSpellInfo.hpp"

namespace ui {

ListMagicSpells::ListMagicSpells(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListMagicSpells::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListMagicSpells::setProps(const ListMagicSpellsProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListMagicSpellsProps& ListMagicSpells::getProps() const { return props; }

UiElement* ListMagicSpells::createSpellElement(
    const ListMagicSpellsPropsSpell& spell) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(spell.id);
  container->setScale(1.0f);
  container->setProps(QuadProps{
      .width = rowWidth,
      .height = rowHeight,
      .bgColor = Colors::Transparent,
  });
  if (props.enableSpellInfoOnClick && !spell.id.empty()) {
    container->addEventObserver(new ObserverShowLayerSpellInfo(window, spell.id));
  }

  // drawSprite uses native sprite w/h (props width/height are ignored), so center
  // and label offset must use the sprite's drawn size, not iconSpriteSize.
  int iconDrawW = 0;
  int iconDrawH = 0;
  if (!spell.iconSprite.empty()) {
    const auto& sprite =
        window->getStore().getSprite(bmin::toStringView(spell.iconSprite));
    iconDrawW = static_cast<int>(sprite.w * iconScale * style.scale);
    iconDrawH = static_cast<int>(sprite.h * iconScale * style.scale);

    auto icon = new SpriteElement(window, this);
    icon->setId("icon");
    icon->setPos(0, (rowHeight - iconDrawH) / 2);
    icon->setScale(iconScale * style.scale);
    icon->setProps(SpriteElementProps{
        .width = sprite.w,
        .height = sprite.h,
        .spriteName = spell.iconSprite,
    });
    container->addChild(icon);
  }

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  const int labelX =
      iconDrawW + static_cast<int>(labelGapAfterIcon * style.scale);

  auto label = new TextLine(window, this);
  label->setId("label");
  label->setPos(labelX, rowHeight / 2);
  label->setScale(1.0f);
  TextLineProps labelProps;
  labelProps.fontFamily = font.fontFamily;
  labelProps.fontSize = sdl2w::TEXT_SIZE_18;
  labelProps.fontColor = Colors::Black;
  labelProps.textAlign = TextAlign::LEFT_CENTER;
  labelProps.textBlocks.pushBack({.text = spell.label});
  label->setProps(labelProps);
  container->addChild(label);

  const int scaledRequiredRuneSize = static_cast<int>(
      requiredRuneIconSize * requiredRuneIconScale * style.scale);
  const int scaledRequiredRuneGap =
      static_cast<int>(requiredRuneGap * style.scale);
  const int scaledRightPadding =
      static_cast<int>(requiredRunesRightPadding * style.scale);
  const int requiredCount = static_cast<int>(spell.requiredRuneSprites.size());
  if (requiredCount > 0) {
    const int totalRequiredWidth =
        requiredCount * scaledRequiredRuneSize +
        (requiredCount - 1) * scaledRequiredRuneGap;
    int runeX = rowWidth - scaledRightPadding - totalRequiredWidth;
    const int runeY = (rowHeight - scaledRequiredRuneSize) / 2;
    for (int i = 0; i < requiredCount; ++i) {
      const auto& runeSprite = spell.requiredRuneSprites[static_cast<size_t>(i)];
      if (runeSprite.empty()) {
        runeX += scaledRequiredRuneSize + scaledRequiredRuneGap;
        continue;
      }
      auto runeIcon = new SpriteElement(window, this);
      runeIcon->setId("requiredRune_" + bmin::toString(i));
      runeIcon->setPos(runeX, runeY);
      runeIcon->setScale(requiredRuneIconScale * style.scale);
      runeIcon->setProps(SpriteElementProps{
          .width = requiredRuneIconSize,
          .height = requiredRuneIconSize,
          .spriteName = runeSprite,
      });
      container->addChild(runeIcon);
      runeX += scaledRequiredRuneSize + scaledRequiredRuneGap;
    }
  }

  return container;
}

void ListMagicSpells::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  if (props.spells.empty()) {
    return;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  list->setPos(style.x,
               style.y + static_cast<int>(props.paddingTop * style.scale));
  list->setScale(1.0f);

  for (size_t i = 0; i < props.spells.size(); i++) {
    list->addChild(createSpellElement(props.spells[i]));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListMagicSpells::render(int dt) { UiElement::render(dt); }

} // namespace ui
