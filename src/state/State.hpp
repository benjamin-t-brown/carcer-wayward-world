#pragma once

#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include "model/instances/MapInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.hpp"
#include "model/templates/UtilityTypes.h"
#include "state/Triggers.hpp"
#include "state/LayerRequest.h"
#include <cmath>
#include <cstdint>

namespace state {

struct UserSettings {
  int fontScale = 0;
  int floatingNotificationDurationMs = 3000;
  /** Scales attack/FX presentation waits. 0.5 = faster, 2 = slower. */
  float actionDelayMult = 1.f;
};

/** Apply UserSettings::actionDelayMult to a base delay/lifetime in ms. */
inline int scaleActionDelayMs(int baseMs, float mult) {
  if (baseMs <= 0) {
    return 0;
  }
  if (mult <= 0.f) {
    return 0;
  }
  const auto scaled =
      static_cast<int>(std::lround(static_cast<double>(baseMs) * static_cast<double>(mult)));
  return scaled < 0 ? 0 : scaled;
}

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
  std::uint64_t floatingNotificationRevision = 0;
  bmin::DynArray<LayerCommand> layerCommands;
  HeldMove heldMove;
  /** HUD / inventory UI selection only — does not drive map movement. */
  bmin::String selectedPartyMemberId;
};

struct State {
  UiState uiState;
  UserSettings settings;
  model::Player player;
  model::World world;
  Triggers triggers;
  // persistent storage for maps
  bmin::Map<bmin::String, model::MapInstance> mapInstances;
  // Dialogue / special-event runner vars (vars.*, once.*, …). tmp.* is session-only
  // and stripped when a conversation ends.
  bmin::Map<bmin::String, bmin::String> specialEventStorage;

  model::TurnMode turnMode = model::TurnMode::TURN_TOWN;

  // Monotonic counter of player movement ticks (world steps + combat rounds × 4).
  int playerMovementCount = 0;

  // One-shot sound names queued by PlaySound; WorldUpdater plays then clears.
  bmin::DynArray<bmin::String> soundsToPlay;
};

} // namespace state
