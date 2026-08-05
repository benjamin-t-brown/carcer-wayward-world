#pragma once

#include "bmin/DynArray.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/instances/World.h"

namespace game {

inline constexpr int PICKUP_PATH_RANGE = 4;

/** True when the active-map tile at (worldX, worldY) is effectively a container. */
bool isActiveMapTileContainer(model::ActiveMap& activeMap,
                              int worldX,
                              int worldY,
                              const db::Database& database);

/** Ground items the character can path to within maxSteps (excludes container tiles). */
bmin::DynArray<model::ItemInstance>
collectItemsWithinPickupRange(model::ActiveMap& activeMap,
                              const model::CharacterInstance& character,
                              int maxSteps,
                              const db::Database& database);

/** Items stored on a specific active-map tile (container contents or ground pile). */
bmin::DynArray<model::ItemInstance>
collectItemsAtActiveMapTile(const model::ActiveMap& activeMap, int worldX, int worldY);

} // namespace game
