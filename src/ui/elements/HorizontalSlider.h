#pragma once

#include "../UiElement.h"
#include "ui/colors.h"

namespace ui {

struct HorizontalSliderProps {
  int minValue = 1;
  int maxValue = 1;
  int value = 1;
  int width = 220;
  int height = 56;
  int sliderBarHeight = 32;
  int indicatorWidth = 24;
  SDL_Color labelColor = Colors::Black;
};

// Boilerplate HorizontalSlider: basic API + static rendering.
class HorizontalSlider : public UiElement {
  HorizontalSliderProps props;
  bool isDraggingIndicator = false;
  void refreshValueUi();
  bool isInSliderTrack(int mouseX, int mouseY) const;
  bool hitIndicator(int mouseX, int mouseY);
  bool hitButton(int mouseX, int mouseY);
  void setValueFromIndicatorMouseX(int mouseX);

public:
  HorizontalSlider(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~HorizontalSlider() override = default;

  void setProps(const HorizontalSliderProps& _props);
  HorizontalSliderProps& getProps();
  const HorizontalSliderProps& getProps() const;
  void increment();
  void decrement();
  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       DynArray<UiElement*> additionalElements = {}) override;
  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
