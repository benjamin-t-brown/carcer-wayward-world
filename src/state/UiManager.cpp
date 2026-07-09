#include "state/UiManager.h"
#include "model/templates/UtilityTypes.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiRemoveFloatingNotification.hpp"

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
