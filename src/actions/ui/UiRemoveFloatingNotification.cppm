module;
#include <utility>
#include <algorithm>

export module carcer.actions.ui.UiRemoveFloatingNotification;
export import carcer.state;

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
