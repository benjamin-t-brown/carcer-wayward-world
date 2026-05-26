#include "model/Character.h"
#include "db/Database.h"
#include "model/UtilityTypes.h"

namespace model {

CharacterPlayer::CharacterPlayer() { id = createRandomId(); }

std::string characterGetSprite(const Character& character, const db::Database* database) {
  const auto& characterTemplate = database->getCharacterTemplate(character.templateName);
  return characterTemplate.spritesheetName + "_" + characterTemplate.spriteOffset;
}

std::string characterPlayerGetSprite(const CharacterPlayer& characterPlayer) {
  return characterPlayer.params.spritesheetName + "_" +
         characterPlayer.params.spriteOffset;
}

bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const std::string& itemId) {
  return characterPlayer.equipment.weapon0Id == itemId ||
         characterPlayer.equipment.weapon1Id == itemId ||
         characterPlayer.equipment.ammoId == itemId ||
         characterPlayer.equipment.hatId == itemId ||
         characterPlayer.equipment.garbId == itemId ||
         characterPlayer.equipment.glovesId == itemId ||
         characterPlayer.equipment.pantsId == itemId ||
         characterPlayer.equipment.shoesId == itemId ||
         characterPlayer.equipment.necklaceId == itemId ||
         characterPlayer.equipment.shieldId == itemId;
}

std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const std::string& itemName) {
  for (const auto& item : characterPlayer.inventory) {
    if (item.itemName == itemName) {
      return item;
    }
  }
  return std::nullopt;
}

void characterPlayerAddItemToInventory(CharacterPlayer& characterPlayer,
                                       const model::ItemTemplate& itemTemplate,
                                       int quantity) {
  auto existingItem =
      characterPlayerFindItemInInventoryByName(characterPlayer, itemTemplate.name);

  if (existingItem.has_value()) {
    // Item exists, increase quantity
    for (auto& item : characterPlayer.inventory) {
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
    characterPlayer.inventory.push_back(newItem);
  }
}

void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const std::string& itemName,
                                                  int quantity) {
  for (auto it = characterPlayer.inventory.begin(); it != characterPlayer.inventory.end();
       ++it) {
    if (it->itemName == itemName) {
      if (it->quantity > quantity) {
        it->quantity -= quantity;
      } else {
        characterPlayer.inventory.erase(it);
      }
      break;
    }
  }
}

int characterGetWeightCarrying(const CharacterPlayer& characterPlayer,
                               const db::Database* database) {
  int weight = 0;
  for (const auto& item : characterPlayer.inventory) {
    const auto& itemTemplate = database->getItemTemplate(item.itemName);
    weight += item.quantity * itemTemplate.weight;
  }
  return weight;
}

int characterGetWeightCapacity(const CharacterPlayer& characterPlayer) {
  return 100; // TODO derive
}

std::vector<ItemInstance>
characterGetNearbyItems(const CharacterPlayer& characterPlayer) {
  std::vector<ItemInstance> items{
      ItemInstance{.id = createRandomId(), .itemName = "PotionHealing", .quantity = 1},
      ItemInstance{.id = createRandomId(), .itemName = "DaggerBronze", .quantity = 1},
      ItemInstance{.id = createRandomId(), .itemName = "ShortSwordBronze", .quantity = 1},
      ItemInstance{.id = createRandomId(), .itemName = "SwordBronze", .quantity = 1},
      ItemInstance{.id = createRandomId(), .itemName = "LongbowOak", .quantity = 1},
      ItemInstance{.id = createRandomId(), .itemName = "ArrowsStone", .quantity = 50},
  };
  return items;
}

} // namespace model
