#pragma once

#include "model/templates/UtilityTypes.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiPushFloatingNotification : public AbstractAction {
  std::string message;
  UiFloatingNotificationType type;

  void act() override {
    auto& localState = *state;
    UiFloatingNotification notification;
    notification.id = model::createRandomId();
    notification.message = message;
    notification.type = type;
    model::timerStructStart(notification.timer,
                            state->settings.floatingNotificationDurationMs);
    localState.uiState.floatingNotifications.push_back(std::move(notification));
  }

public:
  UiPushFloatingNotification(std::string _message, UiFloatingNotificationType _type)
      : message(std::move(_message)), type(_type) {}
};

} // namespace actions

} // namespace state
