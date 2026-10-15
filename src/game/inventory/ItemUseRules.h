#pragma once

#include "bmin/Map.h"
#include "bmin/String.h"
#include "model/instances/MapInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.hpp"
#include "model/templates/Items.h"
#include "model/templates/Maps.h"

namespace db {
class Database;
}

namespace game {

bool isItemUsableInContext(model::ItemUsability usability,
                           bool combatActive,
                           model::MapType mapType);

/** Usability allows now and item has useAbility.abilityName or useSpecialEvent. */
bool isItemUsableNow(const model::ItemTemplate& itemTemplate,
                     bool combatActive,
                     model::MapType mapType);

/** MapType for the party avatar's current map instance (TOWN if unknown). */
model::MapType resolveActiveMapType(
    model::ActiveMap& activeMap,
    bmin::Map<bmin::String, model::MapInstance>& mapInstances,
    const db::Database* database,
    const model::Player& player);

} // namespace game
