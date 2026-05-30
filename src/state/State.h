#pragma once

#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "model/templates/UtilityTypes.h"

namespace state {

struct UserSettings {
  int fontScale = 0;
  int floatingNotificationDurationMs = 3000;
};

enum class UiFloatingNotificationType {
  INFO,
  WARNING,
  ERROR,
};

struct UiFloatingNotification {
  std::string id;
  std::string message;
  UiFloatingNotificationType type;
  model::TimerStruct timer;
};

struct UiState {
  std::vector<UiFloatingNotification> floatingNotifications;
};

struct State {
  UiState uiState;
  UserSettings settings;
  model::Player player;
  model::World world;
};

} // namespace state