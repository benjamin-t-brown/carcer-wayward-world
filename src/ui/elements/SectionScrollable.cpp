#include "SectionScrollable.h"
#include "ButtonScroll.h"
#include "Quad.h"
#include "lib/sdl2w/Logger.h"
#include "ui/colors.h"
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
  addEventObserver(std::make_unique<SectionScrollableScrollWheelObserver>(this));
}

ui::UiElement* SectionScrollable::getInnerQuad() {
  if (children.size() > 0) {
    auto outerQuad = children[0].get();
    if (outerQuad) {
      auto innerQuad = outerQuad->getChildren()[0].get();
      if (innerQuad) {
        return innerQuad;
      }
    }
  }
  return nullptr;
}

int SectionScrollable::calculateContentHeight() {
  auto innerQuad = getInnerQuad();

  if (innerQuad) {
    auto& children = innerQuad->getChildren();
    int maxYPlusHeight = 0;
    for (const auto& child : children) {
      const auto& childStyle = child->getStyle();
      auto childDims = child->getDims();
      int childBottom = childStyle.y + childDims.second;
      if (childBottom > maxYPlusHeight) {
        maxYPlusHeight = childBottom;
      }
    }
    return maxYPlusHeight;
  }
  return 0;
}

void SectionScrollable::updateScrollIndicatorPosition() {
  // Find the scroll indicator by ID
  for (auto& child : children) {
    if (child->getId() == "scrollIndicator") {
      Quad* indicator = dynamic_cast<Quad*>(child.get());
      if (indicator) {
        // Calculate new Y position based on scroll offset
        int contentWidth = style.width - props.scrollBarWidth;
        int availableSpace = style.height - 3 * props.scrollBarWidth;
        int indicatorY = style.y + props.scrollBarWidth;
        
        if (maxScrollOffset > 0 && availableSpace > 0) {
          float scrollRatio = static_cast<float>(scrollOffset) / static_cast<float>(maxScrollOffset);
          indicatorY = style.y + props.scrollBarWidth + static_cast<int>(scrollRatio * availableSpace);
        }
        
        // Update the indicator position
        indicator->updatePosition(style.x + contentWidth, indicatorY);
      }
      break;
    }
  }
}

void SectionScrollable::setProps(const SectionScrollableProps& _props) {
  props = _props;
  build();
}

SectionScrollableProps& SectionScrollable::getProps() { return props; }

const SectionScrollableProps& SectionScrollable::getProps() const { return props; }

void SectionScrollable::scrollUp() {
  if (scrollOffset > 0) {
    scrollOffset -= props.scrollStep; // Scroll step
    if (scrollOffset < 0) {
      scrollOffset = 0;
    }
    auto innerQuad = getInnerQuad();
    if (innerQuad) {
      Quad* quad = dynamic_cast<Quad*>(innerQuad);
      if (quad) {
        LOG(INFO) << "SectionScrollable::scrollUp: scrollOffset: " << scrollOffset
                  << LOG_ENDL;
        quad->updatePosition(0, -scrollOffset);
      }
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
  auto innerQuad = getInnerQuad();
  if (innerQuad) {
    Quad* quad = dynamic_cast<Quad*>(innerQuad);
    if (quad) {
      LOG(INFO) << "SectionScrollable::scrollDown: scrollOffset: " << scrollOffset
                << LOG_ENDL;
      quad->updatePosition(0, -scrollOffset);
    }
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
  auto innerQuad = getInnerQuad();
  if (innerQuad) {
    Quad* quad = dynamic_cast<Quad*>(innerQuad);
    if (quad) {

      quad->updatePosition(0, -scrollOffset);
    }
  }
  // Update indicator position
  updateScrollIndicatorPosition();
}

void SectionScrollable::addChild(std::unique_ptr<UiElement> child) {
  auto innerQuad = getInnerQuad();
  if (innerQuad) {
    innerQuad->getChildren().push_back(std::move(child));
    build();
  } else {
    LOG(ERROR) << "SectionScrollable::addChild: innerQuad not found to add child"
               << LOG_ENDL;
  }
}

void SectionScrollable::build() {
  // Calculate content area width (totalWidth - scrollBarWidth)
  int contentWidth = style.width - props.scrollBarWidth;
  int previousHeight = calculateContentHeight();

  // Create inner quad (scrollable content area)
  auto innerQuad = std::make_unique<Quad>(window);
  ui::BaseStyle innerStyle;
  innerStyle.x = 0;
  innerStyle.y = scrollOffset; // Apply scroll offset
  innerStyle.width = contentWidth;
  innerStyle.height = std::max(previousHeight, style.height);
  innerStyle.scale = style.scale;
  innerQuad->setStyle(innerStyle);
  ui::QuadProps innerProps;
  innerProps.bgColor = Colors::Transparent;
  innerProps.borderColor = Colors::Transparent;
  innerProps.borderSize = 0;
  innerQuad->setProps(innerProps);
  innerQuad->setId("innerQuad");

  // Create outer quad (viewport/clipping area)
  auto outerQuad = std::make_unique<Quad>(window);
  ui::BaseStyle outerStyle;
  outerStyle.x = style.x;
  outerStyle.y = style.y;
  outerStyle.width = contentWidth;
  outerStyle.height = style.height;
  outerStyle.scale = style.scale;
  outerQuad->setStyle(outerStyle);
  ui::QuadProps outerProps;
  outerProps.bgColor = Colors::White;
  outerProps.borderColor = props.borderColor;
  outerProps.borderSize = props.borderSize;
  outerProps.borderColor = props.borderColor;
  outerQuad->setProps(outerProps);
  outerQuad->setId("outerQuad");

  // Create scroll up button
  auto scrollUpButton = std::make_unique<ButtonScroll>(window);
  ui::BaseStyle upButtonStyle;
  upButtonStyle.x = style.x + contentWidth;
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
  scrollUpButton->addEventObserver(
      std::make_unique<SectionScrollableScrollObserver>(this, true));

  // Create scroll down button
  auto scrollDownButton = std::make_unique<ButtonScroll>(window);
  ui::BaseStyle downButtonStyle;
  downButtonStyle.x = style.x + contentWidth;
  downButtonStyle.y = style.y + style.height - props.scrollBarWidth;
  downButtonStyle.width = props.scrollBarWidth;
  downButtonStyle.height = props.scrollBarWidth;
  downButtonStyle.scale = style.scale;
  scrollDownButton->setStyle(downButtonStyle);
  ui::ButtonScrollProps downButtonProps;
  downButtonProps.direction = ScrollDirection::DOWN;
  downButtonProps.isSelected = false;
  scrollDownButton->setProps(downButtonProps);
  scrollDownButton->setId("scrollDownButton");
  scrollDownButton->addEventObserver(
      std::make_unique<SectionScrollableScrollObserver>(this, false));

  // move children from old quad to new quad, recalculate height if necessary
  UiElement* oldInnerQuad = getInnerQuad();
  if (oldInnerQuad) {
    innerStyle.height = std::max(calculateContentHeight(), style.height);
    innerQuad->setStyle(innerStyle);

    for (auto& child : oldInnerQuad->getChildren()) {
      innerQuad->getChildren().push_back(std::move(child));
    }

    oldInnerQuad->getChildren().clear();
  }

  // Calculate maxScrollOffset before creating indicator
  int contentHeight = innerStyle.height; // Inner quad height
  int viewportHeight = style.height;
  maxScrollOffset = std::max(0, contentHeight - viewportHeight);

  // Create scroll indicator (black rectangle showing scroll position)
  auto scrollIndicator = std::make_unique<Quad>(window);
  ui::BaseStyle indicatorStyle;
  indicatorStyle.x = style.x + contentWidth;
  indicatorStyle.width = props.scrollBarWidth;
  indicatorStyle.height = props.scrollBarWidth;
  indicatorStyle.scale = style.scale;
  
  // Calculate indicator Y position based on scroll offset
  int availableSpace = style.height - 2 * props.scrollBarWidth;
  int indicatorY = style.y + props.scrollBarWidth;
  if (maxScrollOffset > 0 && availableSpace > 0) {
    float scrollRatio = static_cast<float>(scrollOffset) / static_cast<float>(maxScrollOffset);
    indicatorY = style.y + props.scrollBarWidth + static_cast<int>(scrollRatio * availableSpace);
  }
  indicatorStyle.y = indicatorY;
  
  scrollIndicator->setStyle(indicatorStyle);
  ui::QuadProps indicatorProps;
  indicatorProps.bgColor = Colors::LIGHT_GREY;
  indicatorProps.borderColor = Colors::Transparent;
  indicatorProps.borderSize = 0;
  scrollIndicator->setProps(indicatorProps);
  scrollIndicator->setId("scrollIndicator");

  children.clear();

  outerQuad->getChildren().push_back(std::move(innerQuad));
  children.push_back(std::move(outerQuad));
  children.push_back(std::move(scrollUpButton));
  children.push_back(std::move(scrollDownButton));
  children.push_back(std::move(scrollIndicator));
}

void SectionScrollable::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, Colors::White);
  UiElement::render(dt);
}

} // namespace ui
