#pragma once

#include "db/Database.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

// Party avatar on the current map, or nullptr.
CharacterInstance* findPartyAvatarOnMap(MapInstance& map, Player& player);

const CharacterInstance* findPartyAvatarOnMap(const MapInstance& map,
                                              const Player& player);

// After a successful step onto (x, y): queue special event or travel on world.
void queueStepTriggersAt(World& world, const MapInstance& map, int x, int y);

// Console examine text: description line(s) plus item labels on the tile.
bmin::String formatExamineMessage(const MapInstance& map,
                                  int x,
                                  int y,
                                  const db::Database& database);

} // namespace model
