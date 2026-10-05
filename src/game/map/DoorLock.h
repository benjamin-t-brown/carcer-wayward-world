#pragma once

#include "bmin/Map.h"
#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/TileInstance.hpp"

namespace game {

// Outcome of bumping a closed door. Blocked leaves the tile and inventory unchanged.
enum class ClosedDoorOpenResult {
  Blocked,
  OpenedSilent,
  OpenedKey,
  OpenedLockpick,
  OpenedBash,
};

// Non-mutating classification of a closed-door bump for UI routing.
enum class ClosedDoorBumpOutcome {
  OpenImmediateSilent,
  OpenImmediateLockpick,
  OpenImmediateBash,
  ConfirmToolUnlock,
  InfoInsufficientTools,
  ConfirmKeyUnlock,
  InfoMissingKey,
};

struct ClosedDoorBumpInfo {
  ClosedDoorBumpOutcome outcome = ClosedDoorBumpOutcome::InfoMissingKey;
  // max(0, lockLevel - party leader trickery); meaningful for tool locks.
  int toolsRequired = 0;
};

// Party leader only (caller passes player.party[0]). Does not mutate door or inventory.
ClosedDoorBumpInfo classifyClosedDoorBump(
    const model::TileInstance& door,
    const model::CharacterPlayer& leader,
    const bmin::Map<bmin::String, bmin::String>& specialEventStorage,
    const db::Database& database);

// Party leader only (caller passes player.party[0]).
// Key and lockpick success clear doorLock and leave tileId unchanged (door stays
// closed until a later bump). Bash and unlocked silent opens set tileId to tileId + 1.
// Keys are not consumed. Lock tools are removed only when the leader holds the full
// cost across stacks.
ClosedDoorOpenResult tryOpenClosedDoor(
    model::TileInstance& door,
    model::CharacterPlayer& leader,
    const bmin::Map<bmin::String, bmin::String>& specialEventStorage,
    const db::Database& database);

} // namespace game
