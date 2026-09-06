module;
#include <cstddef>

export module carcer.game.inventory;

import carcer.model;
import carcer.db;

export namespace game {

model::EquipItemResult toggleEquippedInventoryItem(
    model::CharacterPlayer& character,
    const bmin::String& itemId,
    const db::Database& database);

model::GiveItemResult giveInventoryItem(model::CharacterPlayer& from,
                                        model::CharacterPlayer& to,
                                        const bmin::String& itemId,
                                        int quantity,
                                        const db::Database& database);

int inventoryWeight(const model::CharacterPlayer& character,
                    const db::Database& database);

} // namespace game
