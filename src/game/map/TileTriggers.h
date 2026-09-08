#pragma once

#include "bmin/String.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/MapInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.hpp"
#include <optional>

namespace game {

// Party leader avatar (party[0]) on the active map, or nullptr.
// Independent of UI selection (selectedPartyMemberId).
model::CharacterInstance* findPartyAvatarOnActiveMap(model::ActiveMap& activeMap,
                                                     model::Player& player);

const model::CharacterInstance*
findPartyAvatarOnActiveMap(const model::ActiveMap& activeMap,
                           const model::Player& player);

// Move existing party leader avatar to (x, y) world tiles, or create one from
// party[0] if missing. Returns nullptr if the party is empty.
// When database is set, caches template AI/faction fields on a newly created avatar.
model::CharacterInstance* placePartyAvatarAt(model::ActiveMap& activeMap,
                                             model::Player& player,
                                             int x,
                                             int y,
                                             const db::Database* database = nullptr);

// Character for drop/pickup placement: prefer the given character if present on
// the active map, otherwise the party leader avatar.
const model::CharacterInstance*
findDropCharacterOnActiveMap(const model::ActiveMap& activeMap,
                             const model::Player& player,
                             const bmin::String& characterId);
model::CharacterInstance* findDropCharacterOnActiveMap(model::ActiveMap& activeMap,
                                                       model::Player& player,
                                                       const bmin::String& characterId);

struct StepTriggerResult {
  std::optional<bmin::String> specialEventId;
  std::optional<model::TravelTrigger> travel;
};

// After a successful step onto (x, y) local map coords: resolve the special
// event or travel request. The caller owns queue/state mutation.
StepTriggerResult resolveStepTriggersAt(const model::MapInstance& map, int x, int y);

// While standing on (x, y) local map coords: resolve travel when it requires action.
std::optional<model::TravelTrigger>
resolveActionTravelAtStanding(const model::MapInstance& map, int x, int y);

// Console examine text: tile description, character labels, and item labels.
bmin::String formatExamineMessage(const model::MapInstance& map,
                                  const model::ActiveMap& activeMap,
                                  int worldX,
                                  int worldY,
                                  int localX,
                                  int localY,
                                  const db::Database& database);

} // namespace game
