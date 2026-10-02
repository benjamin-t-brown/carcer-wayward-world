#pragma once

#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/TileInstance.hpp"

namespace game {

// Outcome of bumping a closed door. Blocked leaves the tile and inventory unchanged.
enum class ClosedDoorOpenResult {
  Blocked,
  OpenedSilent,
  OpenedLockpick,
  OpenedBash,
};

// Party leader only (caller passes player.party[0]).
// Success sets tileId to tileId + 1. Keys are not consumed. Lock tools are
// removed only when the leader holds the full cost across stacks.
ClosedDoorOpenResult tryOpenClosedDoor(model::TileInstance& door,
                                       model::CharacterPlayer& leader,
                                       const db::Database& database);

} // namespace game
