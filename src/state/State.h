#pragma once

#include "model/Player.h"
#include "model/UtilityTypes.h"

namespace state {

inline constexpr int kFloatingNotificationDurationMs = 3000;

struct UserSettings {
  int fontScale = 0;
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
  model::Player player;
  UserSettings settings;
  UiState uiState;
};

} // namespace state