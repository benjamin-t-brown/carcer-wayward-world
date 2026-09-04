module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <cmath>

export module carcer.ui.components.ItemInfo;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.ui.TextStyle;
import carcer.ui.colors;
import carcer.ui.elements;
#include "macros.h"

export {

// --- from ui/components/ItemInfo.h ---
namespace ui {

struct ItemInfoProps {
  int width = 0;
  bmin::String description;
  int weight = 0;
  int value = 0;
};

// ItemInfo - description, weight, and value for an item template.
class ItemInfo : public UiElement {
private:
  ItemInfoProps props;

  static constexpr int kVertSpacerHeight = 12;

public:
  ItemInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ItemInfo() override = default;

  void setProps(const ItemInfoProps& _props);
  const ItemInfoProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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
