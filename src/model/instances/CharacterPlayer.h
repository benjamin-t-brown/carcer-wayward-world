#pragma once

#include "model/instances/ItemInstance.h"
#include "model/stats/CharacterStats.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/UtilityTypes.h"
#include <optional>
#include "lib/Types.h"

namespace db {
class Database;
}

namespace model {

struct CharacterPlayerEquipment {
  // these represent ids of items inside the character's inventory
  String weapon0Id;
  String weapon1Id;
  String ammoId;
  String hatId;
  String garbId;
  String glovesId;
  String pantsId;
  String shoesId;
  String necklaceId;
  String shieldId;
};

struct CharacterInventoryItem {
  String itemName;
  String id;
  int quantity;
};

struct CharacterPlayer {
  String instanceId;
  String name;
  String templateName;
  bmin::DynArray<CharacterInventoryItem> inventory;
  CharacterPlayerEquipment equipment;
  CharacterTemplate params;
  CharacterStats stats;
  int currentHp = 0;
  int currentMp = 0;

  CharacterPlayer(const CharacterTemplate& _params = CharacterTemplate(),
                  const bmin::DynArray<CharacterInventoryItem>& _inventory = {},
                  const CharacterPlayerEquipment& _equipment = {}) {
    instanceId = createRandomId();
    params = _params;
    initCharacterStatsFromTemplate(stats, _params);
    currentHp = _params.combat.hp;
    currentMp = _params.combat.mp;
    inventory = _inventory;
    equipment = _equipment;
  }
};

String characterPlayerGetSprite(const CharacterPlayer& characterPlayer);

enum class EquipItemResult {
  EQUIPPED,
  UNEQUIPPED,
  NOT_EQUIPPABLE,
  ITEM_NOT_IN_INVENTORY,
  SLOT_OCCUPIED,
  TWO_HANDED_OFF_HAND,
};

bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const String& itemId);
EquipItemResult characterPlayerToggleEquipItem(CharacterPlayer& characterPlayer,
                                               const String& itemId,
                                               const db::Database& database);
std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const String& itemName);
void characterPlayerAddItemToInventory(CharacterPlayer& characterPlayer,
                                       const model::ItemTemplate& itemTemplate,
                                       int quantity = 1);
void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const String& itemName,
                                                  int quantity = 1);
void characterPlayerRemoveItemFromInventoryById(CharacterPlayer& characterPlayer,
                                                const String& itemId,
                                                int quantity = 1);

enum class GiveItemResult {
  SUCCESS,
  ITEM_NOT_FOUND,
  INVALID_QUANTITY,
  TOO_HEAVY,
};

GiveItemResult characterPlayerGiveInventoryItem(CharacterPlayer& from,
                                                CharacterPlayer& to,
                                                const String& itemId,
                                                int quantity,
                                                const db::Database& database);
bool characterPlayerReorderInventoryItem(CharacterPlayer& characterPlayer,
                                         size_t index,
                                         int direction);
int characterGetWeightCarrying(const CharacterPlayer& characterPlayer,
                               const db::Database* database);
int characterGetWeightCapacity(const CharacterPlayer& characterPlayer);
int characterGetRationSlotCapacity(const CharacterPlayer& characterPlayer,
                                   const db::Database& database);
bmin::DynArray<ItemInstance> characterGetNearbyItems(const CharacterPlayer& characterPlayer);

} // namespace model
