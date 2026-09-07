#include "game/map/CharacterConstruction.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/templates/CharacterTemplate.h"

namespace game {

bool applyCharacterTemplateFromDatabase(model::CharacterInstance& character,
                                        const db::Database& database) {
  if (character.templateName.empty()) {
    return false;
  }
  try {
    const auto& characterTemplate =
        database.getCharacterTemplate(bmin::toStringView(character.templateName));
    model::applyCharacterTemplateToInstance(character, characterTemplate);
    return true;
  } catch (...) {
    return false;
  }
}

} // namespace game
