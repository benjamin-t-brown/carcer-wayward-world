#pragma once

#include "../UiElement.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"

namespace ui {

// SectionScrollable-specific properties
struct SectionScrollableProps {
  int scrollBarWidth = 32;
  SDL_Color borderColor = Colors::Transparent;
  int borderSize = 0;
  int scrollStep = 20;
  int indicatorHeight = 20;
};

// SectionScrollable element - renders a scrollable section with inner/outer quads and
// scroll buttons Uses Position, Size, Scale from BaseStyle
class SectionScrollable : public UiElement {
private:
  SectionScrollableProps props;
  int scrollOffset = 0;    // Current scroll position
  int maxScrollOffset = 0; // Maximum scroll position
  int innerHeightScaled = 0;
  bool isDraggingIndicator = false;

  std::unique_ptr<Quad> outerQuad = nullptr;
  Quad* innerQuad = nullptr;

  void updateScrollIndicatorPosition();
  void scrollFromIndicatorMouseY(int mouseY);
  bool isInScrollTrack(int mouseX, int mouseY) const;
  bool hitScrollIndicator(int mouseX, int mouseY);
  bool hitScrollButton(int mouseX, int mouseY);

public:
  SectionScrollable(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SectionScrollable() override = default;

  // Setters and getters for section-specific properties
  void setProps(const SectionScrollableProps& _props);
  SectionScrollableProps& getProps();
  const SectionScrollableProps& getProps() const;

  std::pair<int, int> getContentDims() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           std::vector<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         std::vector<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       std::vector<UiElement*> additionalElements = {}) override;
  bool checkMouseWheelEvent(int mouseX,
                            int mouseY,
                            int delta,
                            std::vector<UiElement*> additionalElements = {}) override;

  // Scroll methods
  void scrollUp();
  void scrollDown();
  void scrollTo(int offset);

  void addChild(UiElement* child) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
