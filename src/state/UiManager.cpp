module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.state;
import carcer.data;

namespace state {

void updateUiState(State& state, int dt) {
  auto& notifications = state.uiState.floatingNotifications;
  bool removedNotification = false;
  for (size_t i = 0; i < notifications.size();) {
    auto& notification = notifications[i];
    model::timerStructUpdate(notification.timer, dt);
    if (model::timerStructIsComplete(notification.timer)) {
      notifications.erase(i);
      removedNotification = true;
    } else {
      ++i;
    }
  }
  if (removedNotification) {
    ++state.uiState.floatingNotificationRevision;
  }
}

} // namespace state
