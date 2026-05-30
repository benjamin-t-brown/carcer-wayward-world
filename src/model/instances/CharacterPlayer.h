#pragma once

#include "model/instances/ItemInstance.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/UtilityTypes.h"
#include <optional>
#include <string>
#include <vector>

namespace db {
class Database;
}

namespace model {

struct CharacterPlayerEquipment {
  // these represent ids of items inside the character's inventory
  std::string weapon0Id;
  std::string weapon1Id;
  std::string ammoId;
  std::string hatId;
  std::string garbId;
  std::string glovesId;
  std::string pantsId;
  std::string shoesId;
  std::string necklaceId;
  std::string shieldId;
};

struct CharacterInventoryItem {
  std::string itemName;
  std::string id;
  int quantity;
};

struct CharacterPlayer {
  std::string instanceId;
  std::string name;
  std::string templateName;
  std::vector<CharacterInventoryItem> inventory;
  CharacterPlayerEquipment equipment;
  CharacterTemplate params;

  CharacterPlayer(const CharacterTemplate& _params = CharacterTemplate(),
                  const std::vector<CharacterInventoryItem>& _inventory = {},
                  const CharacterPlayerEquipment& _equipment = {}) {
    instanceId = createRandomId();
    params = _params;
    inventory = _inventory;
    equipment = _equipment;
  }
};

std::string characterPlayerGetSprite(const CharacterPlayer& characterPlayer);

enum class EquipItemResult {
  EQUIPPED,
  UNEQUIPPED,
  NOT_EQUIPPABLE,
  ITEM_NOT_IN_INVENTORY,
  SLOT_OCCUPIED,
  TWO_HANDED_OFF_HAND,
};

bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const std::string& itemId);
EquipItemResult characterPlayerToggleEquipItem(CharacterPlayer& characterPlayer,
                                               const std::string& itemId,
                                               const db::Database& database);
std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const std::string& itemName);
void characterPlayerAddItemToInventory(CharacterPlayer& characterPlayer,
                                       const model::ItemTemplate& itemTemplate,
                                       int quantity = 1);
void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const std::string& itemName,
                                                  int quantity = 1);
void characterPlayerRemoveItemFromInventoryById(CharacterPlayer& characterPlayer,
                                                const std::string& itemId,
                                                int quantity = 1);

enum class GiveItemResult {
  SUCCESS,
  ITEM_NOT_FOUND,
  INVALID_QUANTITY,
  TOO_HEAVY,
};

GiveItemResult characterPlayerGiveInventoryItem(CharacterPlayer& from,
                                                CharacterPlayer& to,
                                                const std::string& itemId,
                                                int quantity,
                                                const db::Database& database);
bool characterPlayerReorderInventoryItem(CharacterPlayer& characterPlayer,
                                         size_t index,
                                         int direction);
int characterGetWeightCarrying(const CharacterPlayer& characterPlayer,
                               const db::Database* database);
int characterGetWeightCapacity(const CharacterPlayer& characterPlayer);
std::vector<ItemInstance> characterGetNearbyItems(const CharacterPlayer& characterPlayer);

} // namespace model
