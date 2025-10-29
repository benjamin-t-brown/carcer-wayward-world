#include "model/Character.h"
#include "model/UtilityTypes.h"

namespace model {

CharacterPlayer::CharacterPlayer() : Character(model::createRandomId()) {
}

bool CharacterPlayer::isItemEquippedById(const std::string& itemId) const {
  return equipment.weapon0Id == itemId ||
         equipment.weapon1Id == itemId ||
         equipment.ammoId == itemId ||
         equipment.hatId == itemId ||
         equipment.garbId == itemId ||
         equipment.glovesId == itemId ||
         equipment.pantsId == itemId ||
         equipment.shoesId == itemId ||
         equipment.necklaceId == itemId ||
         equipment.shieldId == itemId;
}

std::optional<CharacterInventoryItem> CharacterPlayer::findItemInInventoryByName(const std::string& itemName) const {
  for (const auto& item : inventory) {
    if (item.itemName == itemName) {
      return item;
    }
  }
  return std::nullopt;
}

void CharacterPlayer::addItemToInventory(const model::ItemTemplate& itemTemplate, int quantity) {
  auto existingItem = findItemInInventoryByName(itemTemplate.name);
  
  if (existingItem.has_value()) {
    // Item exists, increase quantity
    for (auto& item : inventory) {
      if (item.itemName == itemTemplate.name) {
        item.quantity += quantity;
        break;
      }
    }
  } else {
    // Item doesn't exist, add new item
    CharacterInventoryItem newItem;
    newItem.itemName = itemTemplate.name;
    newItem.id = createRandomId();
    newItem.quantity = quantity;
    inventory.push_back(newItem);
  }
}

void CharacterPlayer::removeItemFromInventoryByName(const std::string& itemName, int quantity) {
  for (auto it = inventory.begin(); it != inventory.end(); ++it) {
    if (it->itemName == itemName) {
      if (it->quantity > quantity) {
        // Reduce quantity
        it->quantity -= quantity;
      } else {
        // Remove item completely
        inventory.erase(it);
      }
      break;
    }
  }
}

} // namespace model
