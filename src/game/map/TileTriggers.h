#pragma once

#include "bmin/String.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/MapInstance.h"
#include "model/instances/Player.h"
#include "state/Triggers.h"


namespace game {

// Party avatar on the current map, or nullptr.
model::CharacterInstance* findPartyAvatarOnMap(model::MapInstance& map,
                                               model::Player& player);

const model::CharacterInstance* findPartyAvatarOnMap(const model::MapInstance& map,
                                                     const model::Player& player);

// Move existing party avatar to (x, y), or create one from the current party
// member if missing. Returns nullptr if the party is empty.
model::CharacterInstance*
placePartyAvatarAt(model::MapInstance& map, model::Player& player, int x, int y);

// After a successful step onto (x, y): queue special event or travel.
void queueStepTriggersAt(state::Triggers& triggers,
                         const model::MapInstance& map,
                         int x,
                         int y);

// While standing on (x, y): queue travel when the tile's travel trigger requires action.
void queueActionTravelAtStanding(state::Triggers& triggers,
                                 const model::MapInstance& map,
                                 int x,
                                 int y);

// Console examine text: tile description, character labels, and item labels on the tile.
bmin::String formatExamineMessage(const model::MapInstance& map,
                                  int x,
                                  int y,
                                  const db::Database& database);

} // namespace game
