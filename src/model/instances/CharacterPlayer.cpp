#include "model/instances/CharacterPlayer.h"
#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "model/templates/UtilityTypes.h"
#include <utility>

namespace model {

namespace {

std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryById(const CharacterPlayer& characterPlayer,
                                       const bmin::String& itemId) {
  for (const auto& item : characterPlayer.inventory) {
    if (item.id == itemId) {
      return item;
    }
  }
  return std::nullopt;
}

bmin::String* equipmentSlotForItemType(CharacterPlayerEquipment& equipment,
                                      ItemType itemType) {
  switch (itemType) {
  case ItemType::WEAPON_AMMO:
    return &equipment.ammoId;
  case ItemType::SHIELD:
    return &equipment.shieldId;
  case ItemType::HAT:
    return &equipment.hatId;
  case ItemType::GARB:
    return &equipment.garbId;
  case ItemType::GLOVES:
    return &equipment.glovesId;
  case ItemType::PANTS:
    return &equipment.pantsId;
  case ItemType::SHOES:
    return &equipment.shoesId;
  case ItemType::NECKLACE:
    return &equipment.necklaceId;
  default:
    return nullptr;
  }
}

void unequipMainWeapon(CharacterPlayerEquipment& equipment) {
  if (!equipment.weapon1Id.empty()) {
    equipment.weapon0Id = equipment.weapon1Id;
    equipment.weapon1Id.clear();
  } else {
    equipment.weapon0Id.clear();
  }
}

void unequipWeaponByItemId(CharacterPlayerEquipment& equipment,
                           const bmin::String& itemId) {
  if (equipment.weapon0Id == itemId) {
    unequipMainWeapon(equipment);
  } else if (equipment.weapon1Id == itemId) {
    equipment.weapon1Id.clear();
  }
}

EquipItemResult equipWeapon(CharacterPlayerEquipment& equipment,
                            const bmin::String& itemId,
                            ItemType itemType) {
  const bool isTwoHanded = itemTypeIsTwoHandedWeapon(itemType);

  if (isTwoHanded) {
    if (!equipment.weapon1Id.empty()) {
      return EquipItemResult::SLOT_OCCUPIED;
    }
    if (equipment.weapon0Id.empty()) {
      equipment.weapon0Id = itemId;
      return EquipItemResult::EQUIPPED;
    }
    return EquipItemResult::TWO_HANDED_OFF_HAND;
  }

  if (equipment.weapon0Id.empty()) {
    equipment.weapon0Id = itemId;
    return EquipItemResult::EQUIPPED;
  }
  if (equipment.weapon1Id.empty()) {
    equipment.weapon1Id = itemId;
    return EquipItemResult::EQUIPPED;
  }
  return EquipItemResult::SLOT_OCCUPIED;
}

EquipItemResult equipSingleSlot(CharacterPlayerEquipment& equipment,
                                const bmin::String& itemId,
                                ItemType itemType) {
  auto* slot = equipmentSlotForItemType(equipment, itemType);
  if (slot == nullptr) {
    return EquipItemResult::NOT_EQUIPPABLE;
  }
  if (slot->empty()) {
    *slot = itemId;
    return EquipItemResult::EQUIPPED;
  }
  if (*slot == itemId) {
    return EquipItemResult::EQUIPPED;
  }
  return EquipItemResult::SLOT_OCCUPIED;
}

} // namespace

bmin::String characterPlayerGetSprite(const CharacterPlayer& characterPlayer) {
  return characterPlayer.params.spritesheetName + "_" +
         characterPlayer.params.spriteOffset;
}

std::optional<CharacterEquipmentSlot>
characterPlayerGetEquipmentSlotForItemId(const CharacterPlayer& characterPlayer,
                                         const bmin::String& itemId) {
  const auto& equipment = characterPlayer.equipment;
  if (equipment.weapon0Id == itemId) {
    return CharacterEquipmentSlot::WEAPON0;
  }
  if (equipment.weapon1Id == itemId) {
    return CharacterEquipmentSlot::WEAPON1;
  }
  if (equipment.ammoId == itemId) {
    return CharacterEquipmentSlot::AMMO;
  }
  if (equipment.hatId == itemId) {
    return CharacterEquipmentSlot::HAT;
  }
  if (equipment.garbId == itemId) {
    return CharacterEquipmentSlot::GARB;
  }
  if (equipment.glovesId == itemId) {
    return CharacterEquipmentSlot::GLOVES;
  }
  if (equipment.pantsId == itemId) {
    return CharacterEquipmentSlot::PANTS;
  }
  if (equipment.shoesId == itemId) {
    return CharacterEquipmentSlot::SHOES;
  }
  if (equipment.necklaceId == itemId) {
    return CharacterEquipmentSlot::NECKLACE;
  }
  if (equipment.shieldId == itemId) {
    return CharacterEquipmentSlot::SHIELD;
  }
  return std::nullopt;
}

bmin::String characterEquipmentSlotAbbrev(CharacterEquipmentSlot slot) {
  switch (slot) {
  case CharacterEquipmentSlot::WEAPON0:
    return "m";
  case CharacterEquipmentSlot::WEAPON1:
    return "o";
  case CharacterEquipmentSlot::AMMO:
    return "a";
  case CharacterEquipmentSlot::HAT:
    return "h";
  case CharacterEquipmentSlot::GARB:
    return "b";
  case CharacterEquipmentSlot::GLOVES:
    return "g";
  case CharacterEquipmentSlot::PANTS:
    return "p";
  case CharacterEquipmentSlot::SHOES:
    return "f";
  case CharacterEquipmentSlot::NECKLACE:
    return "n";
  case CharacterEquipmentSlot::SHIELD:
    return "o";
  default:
    return "";
  }
}

bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const bmin::String& itemId) {
  return characterPlayerGetEquipmentSlotForItemId(characterPlayer, itemId).has_value();
}

EquipItemResult characterPlayerToggleEquipItem(CharacterPlayer& characterPlayer,
                                               const bmin::String& itemId,
                                               const db::Database& database) {
  const auto inventoryItem =
      characterPlayerFindItemInInventoryById(characterPlayer, itemId);
  if (!inventoryItem.has_value()) {
    return EquipItemResult::ITEM_NOT_IN_INVENTORY;
  }

  const auto& itemTemplate = database.getItemTemplate(bmin::toStringView(inventoryItem->itemName));
  if (!itemTypeIsEquippable(itemTemplate.itemType)) {
    return EquipItemResult::NOT_EQUIPPABLE;
  }

  if (characterPlayerIsItemEquippedById(characterPlayer, itemId)) {
    if (itemTypeUsesWeaponSlots(itemTemplate.itemType)) {
      unequipWeaponByItemId(characterPlayer.equipment, itemId);
    } else {
      auto* slot =
          equipmentSlotForItemType(characterPlayer.equipment, itemTemplate.itemType);
      if (slot != nullptr) {
        slot->clear();
      }
    }
    return EquipItemResult::UNEQUIPPED;
  }

  if (itemTypeUsesWeaponSlots(itemTemplate.itemType)) {
    return equipWeapon(characterPlayer.equipment, itemId, itemTemplate.itemType);
  }

  return equipSingleSlot(characterPlayer.equipment, itemId, itemTemplate.itemType);
}

std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const bmin::String& itemName) {
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

  if (existingItem.has_value() && itemTemplate.stackable) {
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
    characterPlayer.inventory.pushBack(newItem);
  }
}

bool characterPlayerReorderInventoryItem(CharacterPlayer& characterPlayer,
                                         size_t index,
                                         int direction) {
  auto& inventory = characterPlayer.inventory;
  if (index >= inventory.size()) {
    return false;
  }
  if (direction < 0) {
    if (index == 0) {
      return false;
    }
    std::swap(inventory[index], inventory[index - 1]);
    return true;
  }
  if (direction > 0) {
    if (index + 1 >= inventory.size()) {
      return false;
    }
    std::swap(inventory[index], inventory[index + 1]);
    return true;
  }
  return false;
}

void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const bmin::String& itemName,
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

void unequipItemById(CharacterPlayer& characterPlayer,
                     const bmin::String& itemId,
                     const db::Database& database) {
  const auto inventoryItem =
      characterPlayerFindItemInInventoryById(characterPlayer, itemId);
  if (!inventoryItem.has_value()) {
    return;
  }
  const auto& itemTemplate = database.getItemTemplate(bmin::toStringView(inventoryItem->itemName));
  if (itemTypeUsesWeaponSlots(itemTemplate.itemType)) {
    unequipWeaponByItemId(characterPlayer.equipment, itemId);
  } else {
    auto* slot =
        equipmentSlotForItemType(characterPlayer.equipment, itemTemplate.itemType);
    if (slot != nullptr && *slot == itemId) {
      slot->clear();
    }
  }
}

void characterPlayerRemoveItemFromInventoryById(CharacterPlayer& characterPlayer,
                                                const bmin::String& itemId,
                                                int quantity) {
  for (auto it = characterPlayer.inventory.begin(); it != characterPlayer.inventory.end();
       ++it) {
    if (it->id == itemId) {
      if (it->quantity > quantity) {
        it->quantity -= quantity;
      } else {
        characterPlayer.inventory.erase(it);
      }
      break;
    }
  }
}

GiveItemResult characterPlayerGiveInventoryItem(CharacterPlayer& from,
                                                CharacterPlayer& to,
                                                const bmin::String& itemId,
                                                int quantity,
                                                const db::Database& database) {
  const auto inventoryItem = characterPlayerFindItemInInventoryById(from, itemId);
  if (!inventoryItem.has_value()) {
    return GiveItemResult::ITEM_NOT_FOUND;
  }
  if (quantity < 1 || quantity > inventoryItem->quantity) {
    return GiveItemResult::INVALID_QUANTITY;
  }

  const auto& itemTemplate = database.getItemTemplate(bmin::toStringView(inventoryItem->itemName));
  const int addedWeight = quantity * itemTemplate.weight;
  if (characterGetWeightCarrying(to, &database) + addedWeight >
      characterGetWeightCapacity(to)) {
    return GiveItemResult::TOO_HEAVY;
  }

  if (characterPlayerIsItemEquippedById(from, itemId) &&
      quantity == inventoryItem->quantity) {
    unequipItemById(from, itemId, database);
  }

  characterPlayerRemoveItemFromInventoryById(from, itemId, quantity);
  characterPlayerAddItemToInventory(to, itemTemplate, quantity);
  return GiveItemResult::SUCCESS;
}

int characterGetWeightCarrying(const CharacterPlayer& characterPlayer,
                               const db::Database* database) {
  int weight = 0;
  for (const auto& item : characterPlayer.inventory) {
    const auto& itemTemplate = database->getItemTemplate(bmin::toStringView(item.itemName));
    weight += item.quantity * itemTemplate.weight;
  }
  return weight;
}

int characterGetWeightCapacity(const CharacterPlayer& characterPlayer) {
  return 100; // TODO derive
}

int characterGetRationSlotCapacity(const CharacterPlayer& characterPlayer,
                                   const db::Database& database) {
  constexpr int baseRationSlots = 4;
  return baseRationSlots;
}

bmin::DynArray<ItemInstance>
characterGetNearbyItems(const CharacterPlayer& characterPlayer) {
  // TODO derive
  bmin::DynArray<ItemInstance> items;
  items.pushBack(ItemInstance{
      .id = createRandomId(), .itemTemplateName = "PotionHealing", .quantity = 1});
  items.pushBack(ItemInstance{
      .id = createRandomId(), .itemTemplateName = "DaggerBronze", .quantity = 1});
  items.pushBack(ItemInstance{
      .id = createRandomId(), .itemTemplateName = "ShortSwordBronze", .quantity = 1});
  items.pushBack(ItemInstance{
      .id = createRandomId(), .itemTemplateName = "SwordBronze", .quantity = 1});
  items.pushBack(ItemInstance{
      .id = createRandomId(), .itemTemplateName = "LongbowOak", .quantity = 1});
  items.pushBack(ItemInstance{
      .id = createRandomId(), .itemTemplateName = "ArrowsStone", .quantity = 50});
  return items;
}

} // namespace model
