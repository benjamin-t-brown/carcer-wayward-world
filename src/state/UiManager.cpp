module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.state;
import carcer.actions.ui.UiRemoveFloatingNotification;
import carcer.model.templates;

namespace state {

void UiManager::update(int dt, State& state, StateManager& stateManager) {
  auto& notifications = state.uiState.floatingNotifications;
  bmin::DynArray<bmin::String> expiredIds;

  for (auto& notification : notifications) {
    model::timerStructUpdate(notification.timer, dt);
    if (model::timerStructIsComplete(notification.timer)) {
      expiredIds.pushBack(notification.id);
    }
  }

  for (const auto& id : expiredIds) {
    stateManager.enqueueAction(
        stateManager.getActionData(),
        new actions::UiRemoveFloatingNotification(id),
        0);
  }
}

} // namespace state
