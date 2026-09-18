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
// SET_PORT stores a character id or sprite name; that portrait is used instead
// of the talk owner's when auxName is set.
bmin::String resolveTalkEventPortrait(
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates,
    const bmin::String& auxName = {});

bmin::String resolveTalkEventPortrait(const model::GameEvent& gameEvent,
                                      const db::Database* database,
                                      const bmin::String& auxName = {});

} // namespace game
