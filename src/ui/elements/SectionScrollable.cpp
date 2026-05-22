#include "SectionScrollable.h"
#include "Quad.h"
#include "buttons/ButtonScroll.h"
#include "lib/sdl2w/Logger.h"
#include "ui/colors.h"
#include "ui/uiUtils.h"
#include <algorithm>
#include <memory>

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

  outerQuad = std::make_unique<Quad>(window, this);
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

void SectionScrollable::updateScrollIndicatorPosition() {
  auto elem = getChildById("scrollIndicator");
  if (elem) {
    Quad* indicator = dynamic_cast<Quad*>(elem);
    if (indicator) {
      int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
      int scaledHeight = static_cast<int>(style.height * style.scale);

      // Calculate indicator Y position based on scroll offset
      float availableSpace =
          scaledHeight - (2 * props.scrollBarWidth + props.indicatorHeight) * style.scale;
      int indicatorY = style.y + props.scrollBarWidth * style.scale;
      if (maxScrollOffset > 0 && availableSpace > 0) {
        float scrollRatio =
            static_cast<float>(scrollOffset) / static_cast<float>(maxScrollOffset);
        indicatorY += static_cast<int>(scrollRatio * availableSpace);
      }
      indicator->updatePosition(style.x + scaledContentWidth, indicatorY);
    }
  }
}

void SectionScrollable::setProps(const SectionScrollableProps& _props) {
  props = _props;
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

bool SectionScrollable::checkMouseDownEvent(int mouseX,
                                            int mouseY,
                                            int button,
                                            std::vector<UiElement*> additionalElements) {
  const bool handled =
      UiElement::checkMouseDownEvent(mouseX, mouseY, button, {outerQuad.get()});
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
                                          std::vector<UiElement*> additionalElements) {
  isDraggingIndicator = false;
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button, {outerQuad.get()});
}

bool SectionScrollable::checkHoverEvent(int mouseX,
                                        int mouseY,
                                        std::vector<UiElement*> additionalElements) {
  if (isDraggingIndicator) {
    scrollFromIndicatorMouseY(mouseY);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY, {outerQuad.get()});
}

bool SectionScrollable::checkMouseWheelEvent(int mouseX,
                                             int mouseY,
                                             int delta,
                                             std::vector<UiElement*> additionalElements) {
  return UiElement::checkMouseWheelEvent(mouseX, mouseY, delta, {outerQuad.get()});
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
      innerQuad->updatePosition(0, -scrollOffset);
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
    innerQuad->updatePosition(0, -scrollOffset);
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
    innerQuad->updatePosition(0, -scrollOffset);
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
  innerHeightScaled = 0;
  for (auto& child : innerQuad->getChildren()) {
    innerHeightScaled += child->getDims().second;
  }

  int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
  int scaledContentHeight = innerHeightScaled;
  int scaledHeight = static_cast<int>(style.height * style.scale);

  auto& innerStyle = innerQuad->getStyle();
  innerStyle.x = 0;
  innerStyle.y = scrollOffset * style.scale; // Apply scroll offset
  innerStyle.width = scaledContentWidth;
  innerStyle.height = std::max(scaledContentHeight, scaledHeight);
  innerStyle.scale = 1.f;
  innerQuad->setProps(QuadProps{});

  auto& outerStyle = outerQuad->getStyle();
  outerStyle.x = style.x;
  outerStyle.y = style.y;
  outerStyle.width = scaledContentWidth;
  outerStyle.height = scaledHeight;
  outerStyle.scale = 1.f;
  ui::QuadProps outerProps;
  outerProps.bgColor = Colors::Transparent;
  outerProps.borderColor = props.borderColor;
  outerProps.borderSize = props.borderSize;
  outerProps.borderColor = props.borderColor;
  outerQuad->setProps(outerProps);

  // Create scroll up button
  auto scrollUpButton = new ButtonScroll(window);
  ui::BaseStyle upButtonStyle;
  upButtonStyle.x = style.x + scaledContentWidth;
  upButtonStyle.y = style.y;
  upButtonStyle.width = props.scrollBarWidth;
  upButtonStyle.height = props.scrollBarWidth;
  upButtonStyle.scale = style.scale;
  scrollUpButton->setStyle(upButtonStyle);
  ui::ButtonScrollProps upButtonProps;
  upButtonProps.direction = ScrollDirection::UP;
  upButtonProps.isSelected = false;
  scrollUpButton->setProps(upButtonProps);
  scrollUpButton->setId("scrollUpButton");
  scrollUpButton->addEventObserver(new SectionScrollableScrollObserver(this, true));

  // Create scroll down button
  auto scrollDownButton = new ButtonScroll(window);
  ui::BaseStyle downButtonStyle;
  downButtonStyle.x = style.x + scaledContentWidth;
  downButtonStyle.y = style.y + scaledHeight - props.scrollBarWidth * style.scale;
  downButtonStyle.width = props.scrollBarWidth;
  downButtonStyle.height = props.scrollBarWidth;
  downButtonStyle.scale = style.scale;
  scrollDownButton->setStyle(downButtonStyle);
  ui::ButtonScrollProps downButtonProps;
  downButtonProps.direction = ScrollDirection::DOWN;
  downButtonProps.isSelected = false;
  scrollDownButton->setProps(downButtonProps);
  scrollDownButton->setId("scrollDownButton");
  scrollDownButton->addEventObserver(new SectionScrollableScrollObserver(this, false));

  // Calculate maxScrollOffset before creating indicator
  maxScrollOffset = std::max(0, scaledContentHeight - scaledHeight);

  // Create scroll indicator (rectangle showing scroll position)
  auto scrollIndicator = new Quad(window);
  auto& indicatorStyle = scrollIndicator->getStyle();
  indicatorStyle.x = style.x + scaledContentWidth;
  indicatorStyle.width = props.scrollBarWidth * style.scale;
  indicatorStyle.height = props.indicatorHeight * style.scale;
  indicatorStyle.scale = 1;

  // Calculate indicator Y position based on scroll offset
  float availableSpace =
      scaledHeight - (2 * props.scrollBarWidth + props.indicatorHeight) * style.scale;
  int indicatorY = style.y + props.scrollBarWidth * style.scale;
  if (maxScrollOffset > 0 && availableSpace > 0) {
    float scrollRatio =
        static_cast<float>(scrollOffset) / static_cast<float>(maxScrollOffset);
    indicatorY += static_cast<int>(scrollRatio * availableSpace);
  }
  indicatorStyle.y = indicatorY;
  ui::QuadProps indicatorProps;
  indicatorProps.bgColor = Colors::LightGrey;
  indicatorProps.borderColor = Colors::Transparent;
  indicatorProps.borderSize = 0;
  scrollIndicator->setProps(indicatorProps);
  scrollIndicator->setId("scrollIndicator");

  children.clear();

  UiElement::addChild(scrollUpButton);
  UiElement::addChild(scrollDownButton);
  UiElement::addChild(scrollIndicator);
}

void SectionScrollable::render(int dt) {
  // auto& draw = window->getDraw();
  // auto [scaledWidth, scaledHeight] = getDims();
  // draw.drawRect(style.x, style.y, scaledWidth, scaledHeight, Colors::LightBlue);
  outerQuad->render(dt);
  UiElement::render(dt);
}

} // namespace ui
