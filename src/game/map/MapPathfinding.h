#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/World.h"

namespace game {

struct PathTile {
  int x = 0;
  int y = 0;
  /** Steps from the start tile (start itself is 0). */
  int dist = 0;
};

/**
 * Flood-fill tiles reachable by the given character within maxSteps.
 * 8-directional movement, step cost 1. Includes the start tile.
 * Other characters block tiles; characterId is ignored for occupancy.
 */
bmin::DynArray<PathTile> collectReachableTiles(model::ActiveMap& activeMap,
                                               int startX,
                                               int startY,
                                               int maxSteps,
                                               const bmin::String& characterId,
                                               const db::Database& database);

/** Same as above, using the character's current tile and id. */
bmin::DynArray<PathTile> collectReachableTiles(model::ActiveMap& activeMap,
                                               const model::CharacterInstance& character,
                                               int maxSteps,
                                               const db::Database& database);

bool isTileInReachableSet(const bmin::DynArray<PathTile>& reachable, int x, int y);

} // namespace game
