#pragma once

#include "../UiElement.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include <cstdint>

namespace ui {

struct FloatingNotificationSectionProps {
  // Inset from the top-left of the game/map content area (matches InGameLayout kPad).
  int margin = 4;
  int notificationGap = 8;
  // Top-left of the map content area in screen pixels. Defaults match BorderInGameWide
  // at scale 1 (leftBorderWidth=16, titleHeight=44); LayerWorld updates on align.
  int originX = 16;
  int originY = 44;
};

class FloatingNotificationSection : public UiElement {
private:
  FloatingNotificationSectionProps props;
  // Last synced uiState.floatingNotificationRevision; render polls for changes
  // so inline pushes (not only UiPushFloatingNotification) and timer expiry show up.
  std::uint64_t syncedRevision = 0;

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
  void syncIfRevisionChanged();

public:
  FloatingNotificationSection(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotificationSection() override;

  void setProps(const FloatingNotificationSectionProps& _props);
  const FloatingNotificationSectionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
