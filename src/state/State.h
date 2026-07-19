#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
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
  bmin::String id;
  bmin::String message;
  UiFloatingNotificationType type;
  model::TimerStruct timer;
};

struct HeldMove {
  bool isActive = false;
  bmin::String key;
  int dx = 0;
  int dy = 0;
  model::TimerStruct initialDelay = model::TimerStruct(300);
  model::TimerStruct moveDelay = model::TimerStruct(50);
};

struct UiState {
  bmin::DynArray<UiFloatingNotification> floatingNotifications;
  HeldMove heldMove;
};

struct State {
  UiState uiState;
  UserSettings settings;
  model::Player player;
  model::World world;
};

} // namespace state
