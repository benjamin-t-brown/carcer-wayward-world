#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"
#include <algorithm>

namespace state {

namespace actions {

class UiRemoveFloatingNotification : public AbstractAction {
  String notificationId;

  void act() override {
    auto& notifications = state->uiState.floatingNotifications;
    notifications.erase(
        std::remove_if(notifications.begin(),
                       notifications.end(),
                       [&](const UiFloatingNotification& n) { return n.id == notificationId; }),
        notifications.end());
  }

public:
  explicit UiRemoveFloatingNotification(String _notificationId)
      : notificationId(std::move(_notificationId)) {}
};

} // namespace actions

} // namespace state
