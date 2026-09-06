module;

module carcer.game.map;

import bmin.string_interop;

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
