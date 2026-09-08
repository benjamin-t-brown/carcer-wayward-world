#pragma once

#include "../UiElement.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace ui {

struct FloatingNotificationSectionProps {
  int bottomMargin = 40;
  int notificationGap = 8;
};

class FloatingNotificationSection : public UiElement {
private:
  FloatingNotificationSectionProps props;

  template <state::ActionEvent Event, typename Fn> void subscribeAction(Fn&& fn) {
    if (!hasStateManager()) {
      return;
    }
    getStateManager()->getActionBus().subscribe(
        this,
        Event,
        [fn = std::forward<Fn>(fn)](state::AbstractAction& action, state::State& state) {
          fn(action, state);
        });
  }

  void syncFromState(const state::State& state);
  void layoutNotifications();

public:
  FloatingNotificationSection(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotificationSection() override;

  void setProps(const FloatingNotificationSectionProps& _props);
  const FloatingNotificationSectionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
