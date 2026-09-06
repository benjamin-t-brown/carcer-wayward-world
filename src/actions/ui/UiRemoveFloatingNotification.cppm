module;
#include <utility>

export module carcer.actions.ui:UiRemoveFloatingNotification;
export import carcer.state;

export {

namespace state::actions {

class UiRemoveFloatingNotification : public AbstractAction {
  bmin::String notificationId;

  void act() override {
    const auto oldSize = state->uiState.floatingNotifications.size();
    state->uiState.floatingNotifications.eraseIf(
        [&](const UiFloatingNotification& notification) {
          return notification.id == notificationId;
        });
    if (state->uiState.floatingNotifications.size() != oldSize) {
      ++state->uiState.floatingNotificationRevision;
    }
  }

public:
  explicit UiRemoveFloatingNotification(bmin::String id)
      : notificationId(std::move(id)) {}
};

} // namespace state::actions

} // export
