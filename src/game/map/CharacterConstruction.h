#pragma once

namespace db {
class Database;
}

namespace model {
struct CharacterInstance;
}

namespace game {

/** Apply the instance's named template; false when absent or unknown. */
bool applyCharacterTemplateFromDatabase(model::CharacterInstance& character,
                                        const db::Database& database);

} // namespace game
