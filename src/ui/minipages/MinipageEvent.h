#pragma once

#include "ui/UiElement.h"

namespace ui {

struct MinipageEventProps {};

// MinipageEvent - renders a small event shell using ModalSmall.
class MinipageEvent : public UiElement {
private:
  MinipageEventProps props;

public:
  MinipageEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageEvent() override = default;

  void setProps(const MinipageEventProps& _props);
  MinipageEventProps& getProps();
  const MinipageEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
