#pragma once

#include "bmin/Map.h"
#include "bmin/String.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/SpecialEvents.hpp"

namespace db {
class Database;
}

namespace game {

// TALK events show the first linked character portrait, then the event icon.
bmin::String resolveTalkEventPortrait(
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates);

bmin::String resolveTalkEventPortrait(const model::GameEvent& gameEvent,
                                      const db::Database* database);

} // namespace game
