#pragma once

#include "model/instances/ItemInstance.h"
#include "model/stats/CharacterStats.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/UtilityTypes.h"
#include <optional>
namespace db {
class Database;
}

namespace model {

struct CharacterPlayerEquipment {
  // these represent ids of items inside the character's inventory
  bmin::String weapon0Id;
  bmin::String weapon1Id;
  bmin::String ammoId;
  bmin::String hatId;
  bmin::String garbId;
  bmin::String glovesId;
  bmin::String pantsId;
  bmin::String shoesId;
  bmin::String necklaceId;
  bmin::String shieldId;
};

struct CharacterInventoryItem {
  bmin::String itemName;
  bmin::String id;
  int quantity;
};

struct CharacterPlayer {
  bmin::String instanceId;
  bmin::String name;
  bmin::String templateName;
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

bmin::String characterPlayerGetSprite(const CharacterPlayer& characterPlayer);

enum class EquipItemResult {
  EQUIPPED,
  UNEQUIPPED,
  NOT_EQUIPPABLE,
  ITEM_NOT_IN_INVENTORY,
  SLOT_OCCUPIED,
  TWO_HANDED_OFF_HAND,
};

enum class CharacterEquipmentSlot {
  WEAPON0,
  WEAPON1,
  AMMO,
  HAT,
  GARB,
  GLOVES,
  PANTS,
  SHOES,
  NECKLACE,
  SHIELD,
};

std::optional<CharacterEquipmentSlot>
characterPlayerGetEquipmentSlotForItemId(const CharacterPlayer& characterPlayer,
                                         const bmin::String& itemId);
bmin::String characterEquipmentSlotAbbrev(CharacterEquipmentSlot slot);
bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const bmin::String& itemId);
EquipItemResult characterPlayerToggleEquipItem(CharacterPlayer& characterPlayer,
                                               const bmin::String& itemId,
                                               const db::Database& database);
std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const bmin::String& itemName);
void characterPlayerAddItemToInventory(CharacterPlayer& characterPlayer,
                                       const model::ItemTemplate& itemTemplate,
                                       int quantity = 1);
void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const bmin::String& itemName,
                                                  int quantity = 1);
void characterPlayerRemoveItemFromInventoryById(CharacterPlayer& characterPlayer,
                                                const bmin::String& itemId,
                                                int quantity = 1);

enum class GiveItemResult {
  SUCCESS,
  ITEM_NOT_FOUND,
  INVALID_QUANTITY,
  TOO_HEAVY,
};

GiveItemResult characterPlayerGiveInventoryItem(CharacterPlayer& from,
                                                CharacterPlayer& to,
                                                const bmin::String& itemId,
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
