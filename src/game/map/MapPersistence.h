#pragma once

#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "model/instances/World.h"

namespace game {

// Create a MapInstance for every map template in the database.
MapInstanceStore createMapInstances(const db::Database& database);

// Age tile fields on every MapInstance. Movement-count ownership remains with
// the orchestration caller.
void ageMapInstances(MapInstanceStore& mapInstances, int steps);

// Record a defeated map enemy on the MapInstance under its world position so it
// stays gone when entities are next hoisted into the active map.
void markMapCharacterDefeated(model::ActiveMap& activeMap,
                              MapInstanceStore& mapInstances,
                              const model::CharacterInstance& character,
                              const db::Database& database);

// Resolve which map-grid to load for a travel destination (grid name, or the
// grid that contains the map, or a synthetic 1x1 grid for standalone maps).
bmin::String resolveGridIdForMapOrGrid(db::Database& database,
                                       const bmin::String& mapOrGridName);

} // namespace game
