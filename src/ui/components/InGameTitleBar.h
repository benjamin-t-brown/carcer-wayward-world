#pragma once

#include "../UiElement.h"
#include "bmin/String.h"
#include <string_view>

namespace ui {

struct InGameTitleBarProps {
  bmin::String title = "Title";
  int day = 0;
  int food = 0;
  int ap = 0;
  bool showAp = false;

  int buttonSize = 32;
  int buttonSpacing = 0;
  int sectionSpacing = 12;
  int statSpacing = 16;
};

// InGameTitleBar component - title bar for the in-game layout with menu buttons,
// customizable title text, and day/food/ap stats.
class InGameTitleBar : public UiElement {
private:
  InGameTitleBarProps props;

  int getContentHeight() const;
  int getBarCenterY() const;
  int centerTopY(int elementHeight) const;

public:
  InGameTitleBar(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameTitleBar() override = default;

  void setProps(const InGameTitleBarProps& _props);
  InGameTitleBarProps& getProps();
  const InGameTitleBarProps& getProps() const;

  // void addMenuButtonObserver(UiEventObserver* observer);
  // void addHelpButtonObserver(UiEventObserver* observer);

  UiElement* createStatLineRightAligned(const bmin::String& text, int x, int y);

  void build() override;
  void render(int dt) override;
};

} // namespace ui
