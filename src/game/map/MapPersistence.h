#pragma once

#include "db/Database.h"
#include "model/instances/World.h"
#include "state/State.h"

namespace game {

// Create a MapInstance for every map template in the database and store them on
// state.mapInstances.
void createMapInstances(state::State& state, const db::Database& database);

// Age tile fields on every MapInstance (and bump playerMovementCount).
void advanceWorldMovementTicks(state::State& state, int steps);

// Record a defeated map enemy on the MapInstance under its world position so it
// stays gone when entities are next hoisted into the active map.
void markMapCharacterDefeated(state::State& state,
                              const model::CharacterInstance& character);

// Resolve which map-grid to load for a travel destination (grid name, or the
// grid that contains the map, or a synthetic 1x1 grid for standalone maps).
bmin::String resolveGridIdForMapOrGrid(db::Database& database,
                                       const bmin::String& mapOrGridName);

} // namespace game
