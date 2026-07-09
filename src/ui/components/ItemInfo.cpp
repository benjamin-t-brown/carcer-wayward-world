#include "ItemInfo.h"
#include "lib/sdl2w/L10n.h"
#include "ui/colors.h"
#include <cmath>
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"

namespace ui {

ItemInfo::ItemInfo(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ItemInfo::setProps(const ItemInfoProps& _props) {
  props = _props;
  build();
}

const ItemInfoProps& ItemInfo::getProps() const { return props; }

void ItemInfo::build() {
  children.clear();

  auto description = new TextParagraph(window, this);
  description->setId("description");
  auto& descStyle = description->getStyle();
  descStyle.x = style.x;
  descStyle.y = style.y;
  descStyle.width = style.width;
  descStyle.scale = 1.f;
  setBaseFontConfig(descStyle, BaseFontConfig::MODAL_TEXT);
  descStyle.fontColor = Colors::Black;
  descStyle.textAlign = TextAlign::LEFT_TOP;
  descStyle.lineSpacing = 0;
  TextParagraphProps descProps;
  descProps.textBlocks.pushBack({.text = props.description});
  description->setProps(descProps);
  addChild(description);

  auto [descWidthScaled, descHeightScaled] = description->getDims();
  auto weightLine = new TextLine(window, this);
  weightLine->setId("weight");
  auto& weightStyle = weightLine->getStyle();
  weightStyle.x = style.x;
  weightStyle.y = descStyle.y + descHeightScaled + kVertSpacerHeight * style.scale;
  weightStyle.scale = 1.f;
  setBaseFontConfig(weightStyle, BaseFontConfig::MODAL_TEXT);
  weightStyle.fontColor = Colors::DarkBlue;
  weightStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps weightProps;
  weightProps.textBlocks.pushBack({
      .text = TRANSLATE("Weight: ") + bmin::toString(props.weight) + String(TRANSLATE(" lbs")),
  });
  weightLine->setProps(weightProps);
  addChild(weightLine);
  const auto weightLineHeight = weightLine->getDims().second;

  auto valueLine = new TextLine(window, this);
  valueLine->setId("value");
  auto& valueStyle = valueLine->getStyle();
  valueStyle.x = style.x;
  valueStyle.y = weightStyle.y + weightLineHeight + kVertSpacerHeight * style.scale;
  valueStyle.scale = 1.f;
  setBaseFontConfig(valueStyle, BaseFontConfig::MODAL_TEXT);
  valueStyle.fontColor = Colors::DarkGrey;
  valueStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps valueProps;
  valueProps.textBlocks.pushBack({
      .text = TRANSLATE("Value: ") + bmin::toString(props.value) + String(TRANSLATE(" gp")),
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
