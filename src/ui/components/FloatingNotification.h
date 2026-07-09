#pragma once

#include "../UiElement.h"
#include "state/State.h"

namespace ui {

struct FloatingNotificationProps {
  String id;
  String message;
  state::UiFloatingNotificationType type = state::UiFloatingNotificationType::INFO;
};

class FloatingNotification : public UiElement {
private:
  static constexpr int kHorizontalPadding = 16;
  static constexpr int kVerticalPadding = 8;

  FloatingNotificationProps props;

  SDL_Color getTextColor() const;

public:
  FloatingNotification(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotification() override = default;

  void setProps(const FloatingNotificationProps& _props);
  const FloatingNotificationProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
