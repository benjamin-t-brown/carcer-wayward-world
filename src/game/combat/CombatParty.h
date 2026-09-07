#pragma once

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
                                const db::Database& database);

} // namespace game
