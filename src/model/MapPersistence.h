#pragma once

#include "db/Database.h"
#include "model/instances/World.h"
#include "bmin/String.h"

namespace model {

// Capture live map deltas into persistent state (explored + open doors).
void flushMapInstance(const MapInstance& map,
                      PersistentMapState& persistent,
                      const db::Database& database);

// Apply persistent deltas onto a freshly created MapInstance.
void hydrateMapInstance(MapInstance& map,
                        const PersistentMapState& persistent,
                        const db::Database& database);

// Flush World.currentMap into world.mapsByTemplate[templateName].
void flushCurrentMapToPersistence(World& world, const db::Database& database);

// Hydrate World.currentMap from world.mapsByTemplate if present.
void hydrateCurrentMapFromPersistence(World& world, const db::Database& database);

// Flush current → create from template → hydrate → set currentMap and world name/camera.
void enterMap(World& world, const bmin::String& templateName, const db::Database& database);

} // namespace model
