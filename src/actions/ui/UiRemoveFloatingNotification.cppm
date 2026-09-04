module;
#include <utility>
#include <cstddef>
#include <algorithm>

export module carcer.actions.ui:UiRemoveFloatingNotification;
export import carcer.state;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiRemoveFloatingNotification : public AbstractAction {
  bmin::String notificationId;

  void act() override {
    auto& notifications = state->uiState.floatingNotifications;
    notifications.erase(
        std::remove_if(notifications.begin(),
                       notifications.end(),
                       [&](const UiFloatingNotification& n) { return n.id == notificationId; }),
        notifications.end());
  }

public:
  explicit UiRemoveFloatingNotification(bmin::String _notificationId)
      : notificationId(std::move(_notificationId)) {}
};

} // namespace actions

} // namespace state

} // export
