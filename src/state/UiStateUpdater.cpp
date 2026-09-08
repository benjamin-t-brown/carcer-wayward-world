#include "state/UiStateUpdater.h"
#include "model/templates/UtilityTypes.h"
#include "state/State.hpp"

namespace state {

void UiStateUpdater::update(int dt, State& state) {
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
