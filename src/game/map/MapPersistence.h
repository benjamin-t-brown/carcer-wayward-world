#pragma once

#include "db/Database.h"
#include "model/instances/World.h"

namespace game {

// Create a MapInstance for every map template in the database and store them on
// world.mapInstances.
void createMapInstances(model::World& world, const db::Database& database);

} // namespace game
