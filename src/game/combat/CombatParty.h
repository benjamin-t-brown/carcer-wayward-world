#pragma once

#include "game/map/ActiveMapOrchestrator.h"

namespace db {
class Database;
}

namespace model {
struct Player;
struct World;
}

namespace game {

void addPartyMembersToCombatMap(model::World& world,
                                model::Player& player,
                                MapInstanceStore& mapInstances,
                                const db::Database& database);

} // namespace game
