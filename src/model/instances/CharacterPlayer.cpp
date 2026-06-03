#include "model/instances/CharacterPlayer.h"
#include "db/Database.h"
#include "model/templates/UtilityTypes.h"
#include <utility>

namespace model {

namespace {

std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryById(const CharacterPlayer& characterPlayer,
                                       const std::string& itemId) {
  for (const auto& item : characterPlayer.inventory) {
    if (item.id == itemId) {
      return item;
    }
  }
  return std::nullopt;
}

std::string* equipmentSlotForItemType(CharacterPlayerEquipment& equipment,
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
                           const std::string& itemId) {
  if (equipment.weapon0Id == itemId) {
    unequipMainWeapon(equipment);
  } else if (equipment.weapon1Id == itemId) {
    equipment.weapon1Id.clear();
  }
}

EquipItemResult equipWeapon(CharacterPlayerEquipment& equipment,
                            const std::string& itemId,
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
                                const std::string& itemId,
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

EquipItemResult characterPlayerToggleEquipItem(CharacterPlayer& characterPlayer,
                                               const std::string& itemId,
                                               const db::Database& database) {
  const auto inventoryItem =
      characterPlayerFindItemInInventoryById(characterPlayer, itemId);
  if (!inventoryItem.has_value()) {
    return EquipItemResult::ITEM_NOT_IN_INVENTORY;
  }

  const auto& itemTemplate = database.getItemTemplate(inventoryItem->itemName);
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
    characterPlayer.inventory.push_back(newItem);
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

void unequipItemById(CharacterPlayer& characterPlayer,
                     const std::string& itemId,
                     const db::Database& database) {
  const auto inventoryItem =
      characterPlayerFindItemInInventoryById(characterPlayer, itemId);
  if (!inventoryItem.has_value()) {
    return;
  }
  const auto& itemTemplate = database.getItemTemplate(inventoryItem->itemName);
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
                                                const std::string& itemId,
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
                                                const std::string& itemId,
                                                int quantity,
                                                const db::Database& database) {
  const auto inventoryItem = characterPlayerFindItemInInventoryById(from, itemId);
  if (!inventoryItem.has_value()) {
    return GiveItemResult::ITEM_NOT_FOUND;
  }
  if (quantity < 1 || quantity > inventoryItem->quantity) {
    return GiveItemResult::INVALID_QUANTITY;
  }

  const auto& itemTemplate = database.getItemTemplate(inventoryItem->itemName);
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
    const auto& itemTemplate = database->getItemTemplate(item.itemName);
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

std::vector<ItemInstance>
characterGetNearbyItems(const CharacterPlayer& characterPlayer) {
  // TODO derive
  std::vector<ItemInstance> items{
      ItemInstance{
          .id = createRandomId(), .itemTemplateName = "PotionHealing", .quantity = 1},
      ItemInstance{
          .id = createRandomId(), .itemTemplateName = "DaggerBronze", .quantity = 1},
      ItemInstance{
          .id = createRandomId(), .itemTemplateName = "ShortSwordBronze", .quantity = 1},
      ItemInstance{
          .id = createRandomId(), .itemTemplateName = "SwordBronze", .quantity = 1},
      ItemInstance{
          .id = createRandomId(), .itemTemplateName = "LongbowOak", .quantity = 1},
      ItemInstance{
          .id = createRandomId(), .itemTemplateName = "ArrowsStone", .quantity = 50},
  };
  return items;
}

} // namespace model
