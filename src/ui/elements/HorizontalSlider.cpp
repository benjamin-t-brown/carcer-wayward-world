#include "HorizontalSlider.h"
#include "TextLine.h"
#include "ui/TextStyle.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonScroll.h"
#include "ui/uiUtils.h"
#include <algorithm>

namespace ui {

class HorizontalSliderButtonObserver : public UiEventObserver {
  HorizontalSlider* slider;
  bool incrementDirection;

public:
  HorizontalSliderButtonObserver(HorizontalSlider* _slider, bool _incrementDirection)
      : slider(_slider), incrementDirection(_incrementDirection) {}

  void onClick(int x, int y, int button) override {
    if (!slider) {
      return;
    }
    if (incrementDirection) {
      slider->increment();
    } else {
      slider->decrement();
    }
  }
};

HorizontalSlider::HorizontalSlider(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void HorizontalSlider::setProps(const HorizontalSliderProps& _props) {
  props = _props;
  props.minValue = std::max(1, props.minValue);
  props.maxValue = std::max(props.minValue, props.maxValue);
  props.value = std::clamp(props.value, props.minValue, props.maxValue);
  build();
}

HorizontalSliderProps& HorizontalSlider::getProps() { return props; }

const HorizontalSliderProps& HorizontalSlider::getProps() const { return props; }

void HorizontalSlider::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void HorizontalSlider::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

bool HorizontalSlider::isInSliderTrack(int mouseX, int mouseY) const {
  const int buttonSize = props.sliderBarHeight * style.scale;
  const int scaledWidth = static_cast<int>(props.width * style.scale);
  const int trackX = style.x + buttonSize;
  const int trackW = std::max<int>(0, scaledWidth - buttonSize * 2);
  const int trackH = props.sliderBarHeight * style.scale;
  return isInBounds(mouseX, mouseY, trackX, style.y, trackW, trackH);
}

bool HorizontalSlider::hitIndicator(int mouseX, int mouseY) {
  auto* indicator = getChildById("thumb");
  return indicator != nullptr && isInBoundsScaled(mouseX, mouseY, indicator);
}

bool HorizontalSlider::hitButton(int mouseX, int mouseY) {
  auto* leftButton = getChildById("leftButton");
  auto* rightButton = getChildById("rightButton");
  return (leftButton != nullptr && isInBoundsScaled(mouseX, mouseY, leftButton)) ||
         (rightButton != nullptr && isInBoundsScaled(mouseX, mouseY, rightButton));
}

void HorizontalSlider::setValueFromIndicatorMouseX(int mouseX) {
  const int range = std::max<int>(0, props.maxValue - props.minValue);
  if (range <= 0) {
    props.value = props.minValue;
    refreshValueUi();
    return;
  }

  const int buttonSize = props.sliderBarHeight * style.scale;
  const int scaledWidth = static_cast<int>(props.width * style.scale);
  const int trackX = style.x + buttonSize;
  const int trackW = std::max<int>(0, scaledWidth - buttonSize * 2);
  const int thumbW = props.indicatorWidth * style.scale;
  const int availableSpace = std::max<int>(0, trackW - thumbW);
  if (availableSpace <= 0) {
    return;
  }

  const int thumbHalf = thumbW / 2;
  const int relativeX = mouseX - trackX - thumbHalf;
  const float ratio =
      std::clamp(static_cast<float>(relativeX) / availableSpace, 0.f, 1.f);
  props.value =
      props.minValue + static_cast<int>(ratio * static_cast<float>(range) + 0.5f);
  refreshValueUi();
}

void HorizontalSlider::refreshValueUi() {
  const int buttonSize = props.sliderBarHeight * style.scale;
  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int trackX = style.x + buttonSize;
  const int trackWidth = std::max<int>(0, scaledWidth - buttonSize * 2);
  const int range = std::max<int>(1, props.maxValue - props.minValue);
  const float pct = static_cast<float>(props.value - props.minValue) / range;

  if (auto* thumb = dynamic_cast<Quad*>(getChildById("thumb"))) {
    const int maxThumbOffset =
        std::max<int>(0, trackWidth - props.indicatorWidth * style.scale);
    thumb->setPos(trackX + static_cast<int>(pct * maxThumbOffset), style.y);
  }

  if (auto* valueLabel = dynamic_cast<TextLine*>(getChildById("valueLabel"))) {
    TextLineProps valueProps = valueLabel->getProps();
    valueProps.textBlocks.clear();
    valueProps.textBlocks.pushBack({
        .text = bmin::toString(props.value) + " / " + bmin::toString(props.maxValue),
        .fontColor = props.labelColor,
    });
    valueProps.fontColor = props.labelColor;
    valueLabel->setProps(valueProps);
  }
}

void HorizontalSlider::increment() {
  props.value = std::min(props.maxValue, props.value + 1);
  refreshValueUi();
}

void HorizontalSlider::decrement() {
  props.value = std::max(props.minValue, props.value - 1);
  refreshValueUi();
}

bool HorizontalSlider::checkMouseDownEvent(
    int mouseX, int mouseY, int button, bmin::DynArray<UiElement*> additionalElements) {
  const bool handled = UiElement::checkMouseDownEvent(mouseX, mouseY, button);
  if (isInSliderTrack(mouseX, mouseY) && !hitButton(mouseX, mouseY)) {
    setValueFromIndicatorMouseX(mouseX);
    if (hitIndicator(mouseX, mouseY)) {
      isDraggingIndicator = true;
    }
  }
  return handled;
}

bool HorizontalSlider::checkMouseUpEvent(
    int mouseX, int mouseY, int button, bmin::DynArray<UiElement*> additionalElements) {
  isDraggingIndicator = false;
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button);
}

bool HorizontalSlider::checkHoverEvent(int mouseX,
                                       int mouseY,
                                       bmin::DynArray<UiElement*> additionalElements) {
  if (isDraggingIndicator) {
    setValueFromIndicatorMouseX(mouseX);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY);
}

const std::pair<int, int> HorizontalSlider::getDims() const {
  return {static_cast<int>(props.width * style.scale),
          static_cast<int>(props.height * style.scale)};
}

void HorizontalSlider::build() {
  children.clear();

  style.width = props.width;
  style.height = props.height;

  const int scaledWidth = static_cast<int>(props.width * style.scale);
  const int scaledBarHeight = props.sliderBarHeight * style.scale;
  const int buttonSize = scaledBarHeight;
  const int trackX = style.x + buttonSize;
  const int trackWidth = std::max<int>(0, scaledWidth - buttonSize * 2);

  auto leftButton = new ButtonScroll(window, this);
  leftButton->setId("leftButton");
  leftButton->setPos(style.x, style.y);
  leftButton->setScale(style.scale);
  leftButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::LEFT,
      .width = props.sliderBarHeight,
      .height = props.sliderBarHeight,
  });
  leftButton->addEventObserver(new HorizontalSliderButtonObserver(this, false));
  addChild(leftButton);

  auto rightButton = new ButtonScroll(window, this);
  rightButton->setId("rightButton");
  rightButton->setPos(style.x + scaledWidth - buttonSize, style.y);
  rightButton->setScale(style.scale);
  rightButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::RIGHT,
      .width = props.sliderBarHeight,
      .height = props.sliderBarHeight,
  });
  rightButton->addEventObserver(new HorizontalSliderButtonObserver(this, true));
  addChild(rightButton);

  auto track = new Quad(window, this);
  track->setId("track");
  track->setPos(trackX, style.y);
  track->setScale(1.f);
  track->setProps(QuadProps{
      .width = trackWidth,
      .height = scaledBarHeight,
      .bgColor = Colors::Transparent,
      .borderColor = Colors::Black,
      .borderSize = 0,
  });
  addChild(track);

  auto thumb = new Quad(window, this);
  thumb->setId("thumb");
  thumb->setPos(trackX, style.y);
  thumb->setScale(1.f);
  thumb->setProps(QuadProps{
      .width = props.indicatorWidth * static_cast<int>(style.scale),
      .height = scaledBarHeight,
      .bgColor = Colors::Grey,
      .borderColor = Colors::Black,
      .borderSize = 0,
  });
  addChild(thumb);

  auto valueLabel = new TextLine(window, this);
  valueLabel->setId("valueLabel");
  valueLabel->setPos(style.x + scaledWidth / 2,
                     style.y + scaledBarHeight + static_cast<int>(8 * style.scale));
  valueLabel->setScale(1.f);
  TextLineProps valueLabelProps;
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);
  valueLabelProps.fontFamily = font.fontFamily;
  valueLabelProps.fontSize = font.fontSize;
  valueLabelProps.fontColor = props.labelColor;
  valueLabelProps.textAlign = TextAlign::CENTER;
  valueLabelProps.textBlocks.pushBack({.text = ""});
  valueLabel->setProps(valueLabelProps);
  addChild(valueLabel);
  refreshValueUi();
}

void HorizontalSlider::render(int dt) { UiElement::render(dt); }

} // namespace ui
