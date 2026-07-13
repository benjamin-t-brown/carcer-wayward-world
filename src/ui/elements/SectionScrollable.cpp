#include "SectionScrollable.h"
#include "bmin/UniquePtr.h"
#include "Quad.h"
#include "buttons/ButtonScroll.h"
#include "sdl2w/Logger.h"
#include "ui/colors.h"
#include "ui/uiUtils.h"
#include <algorithm>

namespace ui {

class SectionScrollableScrollObserver : public UiEventObserver {
  SectionScrollable* sectionScrollable;
  bool isUpButton;

public:
  SectionScrollableScrollObserver(SectionScrollable* _sectionScrollable, bool _isUpButton)
      : sectionScrollable(_sectionScrollable), isUpButton(_isUpButton) {}
  ~SectionScrollableScrollObserver() override = default;

  void onMouseDown(int x, int y, int button) override {
    if (isUpButton) {
      sectionScrollable->scrollUp();
    } else {
      sectionScrollable->scrollDown();
    }
  }
  void onMouseUp(int x, int y, int button) override {}
  void onClick(int x, int y, int button) override {}
};

class SectionScrollableScrollWheelObserver : public UiEventObserver {
  SectionScrollable* sectionScrollable;

public:
  SectionScrollableScrollWheelObserver(SectionScrollable* _sectionScrollable)
      : sectionScrollable(_sectionScrollable) {}
  ~SectionScrollableScrollWheelObserver() override = default;

  void onMouseWheel(int x, int y, int delta) override {
    if (delta > 0) {
      sectionScrollable->scrollUp();
    } else {
      sectionScrollable->scrollDown();
    }
  }
};

SectionScrollable::SectionScrollable(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  addEventObserver(new SectionScrollableScrollWheelObserver(this));

  outerQuad = bmin::makeUnique<Quad>(window, this);
  outerQuad->setId("outerQuad");

  innerQuad = new Quad(window, outerQuad.get());
  innerQuad->setId("innerQuad");

  outerQuad->addChild(innerQuad);
  outerQuad->build();

  // dont need to add child for outerQuad, not managed by children
}

std::pair<int, int> SectionScrollable::getContentDims() const {
  return {style.width * style.scale - props.scrollBarWidth * style.scale,
          style.height * style.scale};
}

UiElement* SectionScrollable::getChildById(std::string_view searchId) {
  if (outerQuad) {
    if (auto* found = outerQuad->getChildById(searchId)) {
      return found;
    }
  }
  return UiElement::getChildById(searchId);
}

int SectionScrollable::getScrollIndicatorY(int offset) const {
  const int scaledHeight = static_cast<int>(style.height * style.scale);
  const float availableSpace =
      scaledHeight - (2 * props.scrollBarWidth + props.indicatorHeight) * style.scale;
  int indicatorY = style.y + props.scrollBarWidth * style.scale;
  if (maxScrollOffset > 0 && availableSpace > 0) {
    const float scrollRatio =
        static_cast<float>(offset) / static_cast<float>(maxScrollOffset);
    indicatorY += static_cast<int>(scrollRatio * availableSpace);
  }
  return indicatorY;
}

void SectionScrollable::updateScrollIndicatorPosition() {
  auto elem = getChildById("scrollIndicator");
  if (elem) {
    Quad* indicator = dynamic_cast<Quad*>(elem);
    if (indicator) {
      const int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
      indicator->setPos(style.x + scaledContentWidth,
                        getScrollIndicatorY(scrollOffset));
    }
  }
  updateScrollButtonStates();
}

void SectionScrollable::updateScrollButtonStates() {
  const int currentIndicatorY = getScrollIndicatorY(scrollOffset);

  const int upOffset = std::max(0, scrollOffset - props.scrollStep);
  const bool upDisabled = getScrollIndicatorY(upOffset) == currentIndicatorY;

  const int downOffset = std::min(maxScrollOffset, scrollOffset + props.scrollStep);
  const bool downDisabled = getScrollIndicatorY(downOffset) == currentIndicatorY;

  auto* upButton = dynamic_cast<ButtonScroll*>(getChildById("scrollUpButton"));
  if (upButton && upButton->getProps().isDisabled != upDisabled) {
    ButtonScrollProps upProps = upButton->getProps();
    upProps.isDisabled = upDisabled;
    upButton->setProps(upProps);
  }

  auto* downButton = dynamic_cast<ButtonScroll*>(getChildById("scrollDownButton"));
  if (downButton && downButton->getProps().isDisabled != downDisabled) {
    ButtonScrollProps downProps = downButton->getProps();
    downProps.isDisabled = downDisabled;
    downButton->setProps(downProps);
  }
}

void SectionScrollable::setProps(const SectionScrollableProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

void SectionScrollable::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void SectionScrollable::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

SectionScrollableProps& SectionScrollable::getProps() { return props; }

const SectionScrollableProps& SectionScrollable::getProps() const { return props; }

bool SectionScrollable::isInScrollTrack(int mouseX, int mouseY) const {
  const int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
  const int barX = style.x + scaledContentWidth;
  const int barW = props.scrollBarWidth * style.scale;
  const int trackTop = style.y + props.scrollBarWidth * style.scale;
  const int trackBottom = style.y + static_cast<int>(style.height * style.scale) -
                          props.scrollBarWidth * style.scale;
  return isInBounds(mouseX, mouseY, barX, trackTop, barW, trackBottom - trackTop);
}

bool SectionScrollable::hitScrollIndicator(int mouseX, int mouseY) {
  auto* indicator = getChildById("scrollIndicator");
  return indicator != nullptr && isInBoundsScaled(mouseX, mouseY, indicator);
}

bool SectionScrollable::hitScrollButton(int mouseX, int mouseY) {
  auto* upButton = getChildById("scrollUpButton");
  auto* downButton = getChildById("scrollDownButton");
  return (upButton != nullptr && isInBoundsScaled(mouseX, mouseY, upButton)) ||
         (downButton != nullptr && isInBoundsScaled(mouseX, mouseY, downButton));
}

void SectionScrollable::scrollFromIndicatorMouseY(int mouseY) {
  if (maxScrollOffset <= 0) {
    return;
  }

  const int scaledHeight = static_cast<int>(style.height * style.scale);
  const int trackTop = style.y + props.scrollBarWidth * style.scale;
  const float availableSpace =
      scaledHeight - (2 * props.scrollBarWidth + props.indicatorHeight) * style.scale;
  if (availableSpace <= 0) {
    return;
  }

  const int indicatorHalf = static_cast<int>(props.indicatorHeight * style.scale / 2);
  const int relativeY = mouseY - trackTop - indicatorHalf;
  const float ratio =
      std::clamp(static_cast<float>(relativeY) / availableSpace, 0.f, 1.f);
  scrollTo(static_cast<int>(ratio * maxScrollOffset + 0.5f));
}

namespace {

bmin::DynArray<UiElement*> additionalWithQuad(Quad* quad) {
  bmin::DynArray<UiElement*> elements;
  if (quad != nullptr) {
    elements.pushBack(quad);
  }
  return elements;
}

}  // namespace

bool SectionScrollable::checkMouseDownEvent(int mouseX,
                                            int mouseY,
                                            int button,
                                            bmin::DynArray<UiElement*> additionalElements) {
  const bool handled =
      UiElement::checkMouseDownEvent(mouseX, mouseY, button, additionalWithQuad(outerQuad.get()));
  if (maxScrollOffset > 0 && isInScrollTrack(mouseX, mouseY) &&
      !hitScrollButton(mouseX, mouseY)) {
    scrollFromIndicatorMouseY(mouseY);
    if (hitScrollIndicator(mouseX, mouseY)) {
      isDraggingIndicator = true;
    }
  }
  return handled;
}

bool SectionScrollable::checkMouseUpEvent(int mouseX,
                                          int mouseY,
                                          int button,
                                          bmin::DynArray<UiElement*> additionalElements) {
  isDraggingIndicator = false;
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button, additionalWithQuad(outerQuad.get()));
}

bool SectionScrollable::checkHoverEvent(int mouseX,
                                        int mouseY,
                                        bmin::DynArray<UiElement*> additionalElements) {
  if (isDraggingIndicator) {
    scrollFromIndicatorMouseY(mouseY);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY, additionalWithQuad(outerQuad.get()));
}

bool SectionScrollable::checkMouseWheelEvent(int mouseX,
                                             int mouseY,
                                             int delta,
                                             bmin::DynArray<UiElement*> additionalElements) {
  return UiElement::checkMouseWheelEvent(mouseX, mouseY, delta, additionalWithQuad(outerQuad.get()));
}

void SectionScrollable::scrollUp() {
  if (scrollOffset > 0) {
    scrollOffset -= props.scrollStep; // Scroll step
    if (scrollOffset < 0) {
      scrollOffset = 0;
    }
    if (innerQuad) {
      LOG(INFO) << "SectionScrollable::scrollUp: scrollOffset: " << scrollOffset
                << LOG_ENDL;
      innerQuad->setPos(0, -scrollOffset);
    }
    // Update indicator position
    updateScrollIndicatorPosition();
  }
}

void SectionScrollable::scrollDown() {
  if (scrollOffset < maxScrollOffset) {
    scrollOffset += props.scrollStep; // Scroll step
    if (scrollOffset > maxScrollOffset) {
      scrollOffset = maxScrollOffset;
    }
  }
  if (innerQuad) {
    LOG(INFO) << "SectionScrollable::scrollDown: scrollOffset: " << scrollOffset
              << LOG_ENDL;
    innerQuad->setPos(0, -scrollOffset);
  }
  // Update indicator position
  updateScrollIndicatorPosition();
}

void SectionScrollable::scrollTo(int offset) {
  scrollOffset = offset;
  if (scrollOffset < 0) {
    scrollOffset = 0;
  }
  if (scrollOffset > maxScrollOffset) {
    scrollOffset = maxScrollOffset;
  }
  if (innerQuad) {
    innerQuad->setPos(0, -scrollOffset);
  }
  // Update indicator position
  updateScrollIndicatorPosition();
}

void SectionScrollable::addChild(UiElement* child) {
  if (innerQuad) {
    innerQuad->addChild(child);
  }
}

void SectionScrollable::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  innerHeightScaled = 0;
  for (auto& child : innerQuad->getChildren()) {
    innerHeightScaled += child->getDims().second;
  }

  int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
  int scaledContentHeight = innerHeightScaled;
  int scaledHeight = static_cast<int>(style.height * style.scale);

  innerQuad->setPos(0, -scrollOffset);
  innerQuad->setScale(1.f);
  innerQuad->setProps(QuadProps{
      .width = scaledContentWidth,
      .height = std::max(scaledContentHeight, scaledHeight),
  });

  outerQuad->setPos(style.x, style.y);
  outerQuad->setScale(1.f);
  outerQuad->setProps(QuadProps{
      .width = scaledContentWidth,
      .height = scaledHeight,
      .bgColor = props.bgColor,
      .borderColor = props.borderColor,
      .borderSize = props.borderSize,
  });

  // Create scroll up button
  auto scrollUpButton = new ButtonScroll(window);
  scrollUpButton->setPos(style.x + scaledContentWidth, style.y);
  scrollUpButton->setScale(style.scale);
  scrollUpButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::UP,
      .isSelected = false,
      .width = props.scrollBarWidth,
      .height = props.scrollBarWidth,
  });
  scrollUpButton->setId("scrollUpButton");
  scrollUpButton->addEventObserver(new SectionScrollableScrollObserver(this, true));

  // Create scroll down button
  auto scrollDownButton = new ButtonScroll(window);
  scrollDownButton->setPos(style.x + scaledContentWidth,
                           style.y + scaledHeight - props.scrollBarWidth * style.scale);
  scrollDownButton->setScale(style.scale);
  scrollDownButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::DOWN,
      .isSelected = false,
      .width = props.scrollBarWidth,
      .height = props.scrollBarWidth,
  });
  scrollDownButton->setId("scrollDownButton");
  scrollDownButton->addEventObserver(new SectionScrollableScrollObserver(this, false));

  // Calculate maxScrollOffset before creating indicator
  maxScrollOffset = std::max(0, scaledContentHeight - scaledHeight);

  // Create scroll indicator (rectangle showing scroll position)
  auto scrollIndicator = new Quad(window);
  scrollIndicator->setPos(style.x + scaledContentWidth, getScrollIndicatorY(scrollOffset));
  scrollIndicator->setScale(1.f);
  scrollIndicator->setProps(QuadProps{
      .width = props.scrollBarWidth * static_cast<int>(style.scale),
      .height = props.indicatorHeight * static_cast<int>(style.scale),
      .bgColor = Colors::LightGrey,
      .borderColor = Colors::Transparent,
      .borderSize = 0,
  });
  scrollIndicator->setId("scrollIndicator");

  children.clear();

  UiElement::addChild(scrollUpButton);
  UiElement::addChild(scrollDownButton);
  UiElement::addChild(scrollIndicator);

  updateScrollButtonStates();
}

void SectionScrollable::render(int dt) {
  // auto& draw = window->getDraw();
  // auto [scaledWidth, scaledHeight] = getDims();
  // draw.drawRect(style.x, style.y, scaledWidth, scaledHeight, Colors::LightBlue);
  outerQuad->render(dt);
  UiElement::render(dt);
}

} // namespace ui
