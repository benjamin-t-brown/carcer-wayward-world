#pragma once

#include "model/instances/CharacterPlayer.h"

namespace db {
class Database;
}

namespace game {

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

// True if adding quantity of itemTemplate would not exceed weight capacity.
bool canAddItemToInventory(const model::CharacterPlayer& character,
                           const model::ItemTemplate& itemTemplate,
                           int quantity,
                           const db::Database& database);

} // namespace game
