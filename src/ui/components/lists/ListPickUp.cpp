#include "ListPickUp.h"
#include "ui/elements/VerticalList.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/observers/ObserverPickUpItem.hpp"
#include "ui/observers/ObserverShowLayerPickUpContext.hpp"
#include <algorithm>

namespace ui {

ListPickUp::ListPickUp(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

const std::pair<int, int> ListPickUp::getDims() const {
  int paddingHeight =
      static_cast<int>((props.paddingTop + props.paddingBottom) * style.scale);
  if (children.empty()) {
    return {style.width, paddingHeight};
  }

  auto [listWidth, listHeight] = children[0]->getDims();
  return {listWidth, listHeight + paddingHeight};
}

void ListPickUp::setProps(const ListPickUpProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ListPickUpProps& ListPickUp::getProps() const { return props; }

bmin::String ListPickUp::shortcutLetterForIndex(int index) {
  if (index < 0 || index >= maxShortcutItems) {
    return {};
  }
  return bmin::String(1, static_cast<char>('a' + index));
}

UiElement* ListPickUp::createItemElement(const ListPickUpPropsItem& listItem,
                                         int index) {
  const int rowWidth = static_cast<int>(style.width * style.scale);
  const int rowHeight = static_cast<int>(props.lineHeight * style.scale);

  auto container = new Quad(window, this);
  container->setId(listItem.item.itemTemplateName);
  container->setScale(1.0f);
  container->setProps(QuadProps{
      .width = rowWidth,
      .height = rowHeight,
      .bgColor = Colors::Transparent,
  });

  const int iconWidth = static_cast<int>(iconSpriteSize * iconScale * style.scale);
  auto icon = new Quad(window, this);
  icon->setId("icon");
  icon->setPos(0, (rowHeight - iconWidth) / 2);
  icon->setScale(iconScale * style.scale);
  icon->setProps(QuadProps{
      .width = iconSpriteSize,
      .height = iconSpriteSize,
      .bgColor = {66, 202, 253, 50},
      .bgSprite = listItem.itemSprite,
  });
  container->addChild(icon);

  const int weightRightPadding = 8;
  const int labelWeightGap = 8;
  const int scaledContextBtnSize = contextBtnSize * style.scale;

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  int labelX =
      iconWidth + static_cast<int>(labelGapAfterIconNoShortcut * style.scale);
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
    const int shortcutX =
        iconWidth + static_cast<int>(shortcutGapAfterIcon * style.scale);
    shortcutText->setPos(shortcutX, rowHeight / 2);
    container->addChild(shortcutText);
    labelX = shortcutX + shortcutText->getDims().first +
             static_cast<int>(labelGapAfterShortcut * style.scale);
  }

  auto contextBtn = new ButtonModal(window, this);
  contextBtn->setId("contextBtn");
  contextBtn->setPos(rowWidth - scaledContextBtnSize,
                     (rowHeight - scaledContextBtnSize) / 2);
  contextBtn->setScale(1.0f);
  contextBtn->setProps(ButtonModalProps{
      .text = "?",
      .width = scaledContextBtnSize,
      .height = scaledContextBtnSize,
      .bgColor = Colors::Transparent,
      .bgColorTopRight = Colors::Transparent,
      .bgColorBottomLeft = Colors::Transparent,
      .fontColor = Colors::DarkBlue,
  });
  contextBtn->addEventObserver(
      new ui::ObserverShowLayerPickUpContext(window, listItem.item));
  container->addChild(contextBtn);

  auto weightText = new TextLine(window, this);
  weightText->setId("weightText");
  weightText->setPos(0, rowHeight / 2);
  weightText->setScale(1.0f);
  TextLineProps weightProps;
  weightProps.fontFamily = font.fontFamily;
  weightProps.fontSize = font.fontSize;
  weightProps.fontColor = Colors::DarkGrey;
  weightProps.textAlign = TextAlign::LEFT_CENTER;
  weightProps.textBlocks.pushBack({
      .text = bmin::toString(listItem.item.quantity * listItem.weight) + " lbs",
  });
  weightText->setProps(weightProps);
  auto [weightWidth, _] = weightText->getDims();
  const int weightX = rowWidth - scaledContextBtnSize - weightWidth - weightRightPadding;
  weightText->setPos(weightX, rowHeight / 2);
  container->addChild(weightText);

  const int labelWidth = static_cast<int>(weightX - labelX - labelWeightGap);
  auto label = new ButtonTextWrap(window, this);
  label->setId("label");
  label->setPos(labelX, 0);
  label->setScale(1.0f);
  label->setProps(ButtonTextWrapProps{
      .textParagraph =
          {
              .textBlocks = {{.text = listItem.item.itemTemplateName}},
              .width = labelWidth,
              .fontFamily = font.fontFamily,
              .fontSize = sdl2w::TEXT_SIZE_18,
              .fontColor = Colors::Black,
          },
  });
  const int textOnlyHeight = label->getDims().second;
  const int verticalPadding =
      std::max(0, static_cast<int>((rowHeight - textOnlyHeight) / 2));
  label->setProps(ButtonTextWrapProps{
      .verticalPadding = verticalPadding,
      .textParagraph =
          {
              .textBlocks = {{.text = listItem.itemLabel}},
              .width = labelWidth,
              .fontFamily = font.fontFamily,
              .fontSize = sdl2w::TEXT_SIZE_18,
              .fontColor = Colors::Black,
          },
  });
  label->addEventObserver(new ui::ObserverPickUpItem(listItem.item));
  container->addChild(label);

  return container;
}

void ListPickUp::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  if (props.items.empty()) {
    return;
  }

  auto list = new VerticalList(window, this);
  list->setId("list");
  list->setPos(style.x, style.y + static_cast<int>(props.paddingTop * style.scale));
  list->setScale(1.0f);

  for (size_t i = 0; i < props.items.size(); i++) {
    list->addChild(createItemElement(props.items[i], static_cast<int>(i)));
  }

  list->setProps(VerticalListProps{
      .width = static_cast<int>(style.width * style.scale),
      .lineHeight = static_cast<int>(props.lineHeight * style.scale),
      .lineGap = static_cast<int>(props.lineGap * style.scale),
      .bgColor = Colors::Transparent,
  });

  addChild(list);
}

void ListPickUp::render(int dt) { UiElement::render(dt); }

} // namespace ui
