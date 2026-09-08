#pragma once

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include <algorithm>

namespace state {

namespace actions {

class UiRemoveFloatingNotification : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiRemoveFloatingNotification; }
  bmin::String notificationId;

  void act() override {
    auto& notifications = state->uiState.floatingNotifications;
    const auto previousSize = notifications.size();
    notifications.erase(
        std::remove_if(notifications.begin(),
                       notifications.end(),
                       [&](const UiFloatingNotification& n) { return n.id == notificationId; }),
        notifications.end());
    if (notifications.size() != previousSize) {
      ++state->uiState.floatingNotificationRevision;
    }
  }

public:
  explicit UiRemoveFloatingNotification(bmin::String _notificationId)
      : notificationId(std::move(_notificationId)) {}
};

} // namespace actions

} // namespace state
