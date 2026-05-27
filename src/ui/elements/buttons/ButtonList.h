#pragma once

#include "../../UiElement.h"
#include "ButtonScroll.h"
#include "ui/colors.h"
#include <optional>
#include <string>

namespace ui {

struct ButtonListProps {
  std::string text;
  std::optional<ScrollDirection> arrow;
  bool isSelected = false;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;
  SDL_Color arrowColor = Colors::White;
};

// ButtonList - square button for list rows (modal styling for now)
class ButtonList : public UiElement {
private:
  ButtonListProps props;
  bool isInActiveMode = false;

public:
  static constexpr int defaultLogicalSize = 14;

  bool isActive = false;
  ButtonList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonList() override = default;

  static int yForListRow(int rowHeight, int btnLogicalSize, float scale);

  void setProps(const ButtonListProps& _props);
  ButtonListProps& getProps();
  const ButtonListProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
