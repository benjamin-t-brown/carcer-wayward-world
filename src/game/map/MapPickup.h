#pragma once

#include "bmin/DynArray.h"
#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/ItemInstance.hpp"
#include "model/instances/World.hpp"

namespace game {

inline constexpr int PICKUP_PATH_RANGE = 4;

/** True when the active-map tile at (worldX, worldY) is effectively a container. */
bool isActiveMapTileContainer(model::ActiveMap& activeMap,
                              MapInstanceStore& mapInstances,
                              int worldX,
                              int worldY,
                              const db::Database& database);

/** Ground items the character can path to within maxSteps (excludes container tiles). */
bmin::DynArray<model::ItemInstance>
collectItemsWithinPickupRange(model::ActiveMap& activeMap,
                              MapInstanceStore& mapInstances,
                              const model::CharacterInstance& character,
                              int maxSteps,
                              const db::Database& database);

/** Items stored on a specific active-map tile (container contents or ground pile). */
bmin::DynArray<model::ItemInstance>
collectItemsAtActiveMapTile(const model::ActiveMap& activeMap, int worldX, int worldY);

} // namespace game
