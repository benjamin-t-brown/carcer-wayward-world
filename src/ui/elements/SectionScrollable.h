#pragma once

#include "../UiElement.h"
#include "ui/colors.h"

namespace ui {

// SectionScrollable-specific properties
struct SectionScrollableProps {
  int scrollBarWidth = 20;
  SDL_Color borderColor = Colors::Transparent;
  int borderSize = 0;
  int scrollStep = 20;
};

// SectionScrollable element - renders a scrollable section with inner/outer quads and
// scroll buttons Uses Position, Size, Scale from BaseStyle
class SectionScrollable : public UiElement {
private:
  SectionScrollableProps props;
  int scrollOffset = 0;    // Current scroll position
  int maxScrollOffset = 0; // Maximum scroll position

  ui::UiElement* getInnerQuad();
  int calculateContentHeight();

public:
  SectionScrollable(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SectionScrollable() override = default;

  // Setters and getters for section-specific properties
  void setProps(const SectionScrollableProps& _props);
  SectionScrollableProps& getProps();
  const SectionScrollableProps& getProps() const;

  // Scroll methods
  void scrollUp();
  void scrollDown();
  void scrollTo(int offset);

  void addChild(std::unique_ptr<UiElement> child);

  void build() override;
  void render(int dt) override;
};

} // namespace ui
