#include "ListSpellAllyTargets.h"
#include "bmin/StringInterop.h"
#include "ui/colors.hpp"
#include "ui/elements/Quad.h"
#include "ui/elements/SpriteElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/VerticalList.h"
#include "ui/observers/ActionObserver.hpp"
#include "actions/navigation/UiSelectSpellAllyTarget.hpp"

namespace ui {

ListSpellAllyTargets::ListSpellAllyTargets(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListSpellAllyTargets::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListSpellAllyTargets::setProps(const ListSpellAllyTargetsProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListSpellAllyTargetsProps& ListSpellAllyTargets::getProps() const { return props; }

bmin::String ListSpellAllyTargets::shortcutLetterForIndex(int index) {
  if (index < 0 || index >= maxShortcutItems) {
    return {};
  }
  return bmin::String(1, static_cast<char>('a' + index));
}

UiElement* ListSpellAllyTargets::createAllyElement(const ListSpellAllyTargetsEntry& ally,
                                                   int index) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(ally.id);
  container->setScale(1.0f);
  container->setProps(QuadProps{
      .width = rowWidth,
      .height = rowHeight,
      .bgColor = Colors::Transparent,
  });
  if (!ally.id.empty() && !props.spellId.empty()) {
    container->playClickSound = true;
    container->addEventObserver(
        ui::makeActionObserver<state::actions::UiSelectSpellAllyTarget>(
            props.spellId, props.casterId, ally.id));
  }

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  int contentX = 0;
  const auto shortcutLetter = shortcutLetterForIndex(index);
  if (!shortcutLetter.empty()) {
    auto shortcutText = new TextLine(window, this);
    shortcutText->setId("shortcut");
    shortcutText->setPos(0, rowHeight / 2);
    shortcutText->setScale(1.0f);
    TextLineProps shortcutProps;
    shortcutProps.fontFamily = font.fontFamily;
    shortcutProps.fontSize = sdl2w::TEXT_SIZE_14;
    shortcutProps.fontColor = Colors::Grey;
    shortcutProps.textAlign = TextAlign::LEFT_CENTER;
    shortcutProps.textBlocks.pushBack({.text = shortcutLetter});
    shortcutText->setProps(shortcutProps);
    container->addChild(bmin::UniquePtr<ui::UiElement>(shortcutText));
    contentX = static_cast<int>(shortcutColumnWidth * style.scale) +
               static_cast<int>(shortcutGapAfterLetter * style.scale);
  }

  int iconDrawW = 0;
  int iconDrawH = 0;
  if (!ally.iconSprite.empty()) {
    const auto& sprite =
        window->getStore().getSprite(bmin::toStringView(ally.iconSprite));
    iconDrawW = static_cast<int>(sprite.w * iconScale * style.scale);
    iconDrawH = static_cast<int>(sprite.h * iconScale * style.scale);

    auto icon = new SpriteElement(window, this);
    icon->setId("icon");
    icon->setPos(contentX, (rowHeight - iconDrawH) / 2);
    icon->setScale(iconScale * style.scale);
    icon->setProps(SpriteElementProps{
        .width = sprite.w,
        .height = sprite.h,
        .spriteName = ally.iconSprite,
    });
    container->addChild(bmin::UniquePtr<ui::UiElement>(icon));
  }

  const int labelX =
      contentX + iconDrawW + static_cast<int>(labelGapAfterIcon * style.scale);

  auto label = new TextLine(window, this);
  label->setId("label");
  label->setPos(labelX, rowHeight / 2);
  label->setScale(1.0f);
  TextLineProps labelProps;
  labelProps.fontFamily = font.fontFamily;
  labelProps.fontSize = sdl2w::TEXT_SIZE_18;
  labelProps.fontColor = Colors::Black;
  labelProps.textAlign = TextAlign::LEFT_CENTER;
  labelProps.textBlocks.pushBack({.text = ally.label});
  label->setProps(labelProps);
  container->addChild(bmin::UniquePtr<ui::UiElement>(label));

  return container;
}

void ListSpellAllyTargets::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  if (props.allies.empty()) {
    return;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  list->setPos(style.x, style.y + static_cast<int>(props.paddingTop * style.scale));
  list->setScale(1.0f);

  for (size_t i = 0; i < props.allies.size(); i++) {
    list->addChild(bmin::UniquePtr<ui::UiElement>(
        createAllyElement(props.allies[i], static_cast<int>(i))));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(bmin::UniquePtr<ui::UiElement>(list));
}

void ListSpellAllyTargets::render(int dt) { UiElement::render(dt); }

} // namespace ui
