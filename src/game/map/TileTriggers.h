#pragma once

#include "bmin/Map.h"
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

// Empty condition is true. A present in3 condition is evaluated against storage.
// A passing ONCE() commits its key. An invalid condition does not run the event.
bool tileEventShouldRun(const model::TileEventTrigger& trigger,
                        bmin::Map<bmin::String, bmin::String>& storage);

// Same condition as tileEventShouldRun, but ONCE() only reads storage.
// Used to decide whether a SHOW_EVENT_ON_TILE overlay is visible.
bool tileEventConditionHolds(const model::TileEventTrigger& trigger,
                             const bmin::Map<bmin::String, bmin::String>& storage);

// After a successful step onto (x, y) local map coords: resolve the special
// event or travel request. The caller owns queue/state mutation.
// A step event whose condition is false does not run, and travel on that tile
// can still fire.
StepTriggerResult resolveStepTriggersAt(const model::MapInstance& map,
                                        int x,
                                        int y,
                                        bmin::Map<bmin::String, bmin::String>& storage);

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
