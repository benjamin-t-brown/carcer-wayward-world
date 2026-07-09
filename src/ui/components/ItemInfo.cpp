#include "ItemInfo.h"
#include "sdl2w/L10n.h"
#include "ui/colors.h"
#include <cmath>
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"

namespace ui {

ItemInfo::ItemInfo(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ItemInfo::setProps(const ItemInfoProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ItemInfoProps& ItemInfo::getProps() const { return props; }

void ItemInfo::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  auto description = new TextParagraph(window, this);
  description->setId("description");
  description->setPos(style.x, style.y);
  description->setScale(1.f);
  TextParagraphProps descProps;
  descProps.width = style.width;
  descProps.fontFamily = font.fontFamily;
  descProps.fontSize = font.fontSize;
  descProps.fontColor = Colors::Black;
  descProps.textAlign = TextAlign::LEFT_TOP;
  descProps.lineSpacing = 0;
  descProps.textBlocks.pushBack({.text = props.description});
  description->setProps(descProps);
  addChild(description);

  auto [descWidthScaled, descHeightScaled] = description->getDims();
  auto weightLine = new TextLine(window, this);
  weightLine->setId("weight");
  weightLine->setPos(style.x,
                     style.y + descHeightScaled +
                         static_cast<int>(kVertSpacerHeight * style.scale));
  weightLine->setScale(1.f);
  TextLineProps weightProps;
  weightProps.fontFamily = font.fontFamily;
  weightProps.fontSize = font.fontSize;
  weightProps.fontColor = Colors::DarkBlue;
  weightProps.textAlign = TextAlign::LEFT_TOP;
  weightProps.textBlocks.pushBack({
      .text = TRANSLATE("Weight: ") + bmin::toString(props.weight) +
              bmin::String(TRANSLATE(" lbs")),
  });
  weightLine->setProps(weightProps);
  addChild(weightLine);
  const auto weightLineHeight = weightLine->getDims().second;

  auto valueLine = new TextLine(window, this);
  valueLine->setId("value");
  valueLine->setPos(style.x,
                    style.y + descHeightScaled +
                        static_cast<int>(kVertSpacerHeight * style.scale) +
                        weightLineHeight +
                        static_cast<int>(kVertSpacerHeight * style.scale));
  valueLine->setScale(1.f);
  TextLineProps valueProps;
  valueProps.fontFamily = font.fontFamily;
  valueProps.fontSize = font.fontSize;
  valueProps.fontColor = Colors::DarkGrey;
  valueProps.textAlign = TextAlign::LEFT_TOP;
  valueProps.textBlocks.pushBack({
      .text = TRANSLATE("Value: ") + bmin::toString(props.value) +
              bmin::String(TRANSLATE(" gp")),
  });
  valueLine->setProps(valueProps);
  addChild(valueLine);

  const auto valueLineHeight = valueLine->getDims().second;
  const int contentHeightPx = descHeightScaled +
                              static_cast<int>(kVertSpacerHeight * style.scale) +
                              weightLineHeight +
                              static_cast<int>(kVertSpacerHeight * style.scale) +
                              valueLineHeight;
  style.height = std::max(1, static_cast<int>(std::ceil(contentHeightPx / style.scale)));
}

void ItemInfo::render(int dt) { UiElement::render(dt); }

} // namespace ui
