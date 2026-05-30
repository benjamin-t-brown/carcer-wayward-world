#pragma once

#include "../UiElement.h"
#include <string>

namespace ui {

struct InGameTitleBarProps {
  std::string title = "Title";
  int day = 0;
  int food = 0;
  int ap = 0;
  bool showAp = false;

  int buttonSize = 28;
  int buttonSpacing = 4;
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

  UiElement* createStatLineRightAligned(std::string_view text, int x, int y);

  void build() override;
  void render(int dt) override;
};

} // namespace ui
