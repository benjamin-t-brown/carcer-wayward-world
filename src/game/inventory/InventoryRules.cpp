#include "game/inventory/InventoryRules.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include <optional>

namespace game {
namespace {

std::optional<model::CharacterInventoryItem>
findInventoryItemById(const model::CharacterPlayer& character,
                      const bmin::String& itemId) {
  for (const auto& item : character.inventory) {
    if (item.id == itemId) {
      return item;
    }
  }
  return std::nullopt;
}

bmin::String* equipmentSlotForItemType(model::CharacterPlayerEquipment& equipment,
                                       model::ItemType itemType) {
  switch (itemType) {
  case model::ItemType::WEAPON_AMMO:
    return &equipment.ammoId;
  case model::ItemType::SHIELD:
    return &equipment.shieldId;
  case model::ItemType::HAT:
    return &equipment.hatId;
  case model::ItemType::GARB:
    return &equipment.garbId;
  case model::ItemType::GLOVES:
    return &equipment.glovesId;
  case model::ItemType::PANTS:
    return &equipment.pantsId;
  case model::ItemType::SHOES:
    return &equipment.shoesId;
  case model::ItemType::NECKLACE:
    return &equipment.necklaceId;
  default:
    return nullptr;
  }
}

void unequipMainWeapon(model::CharacterPlayerEquipment& equipment) {
  if (!equipment.weapon1Id.empty()) {
    equipment.weapon0Id = equipment.weapon1Id;
    equipment.weapon1Id.clear();
  } else {
    equipment.weapon0Id.clear();
  }
}

void unequipWeaponByItemId(model::CharacterPlayerEquipment& equipment,
                           const bmin::String& itemId) {
  if (equipment.weapon0Id == itemId) {
    unequipMainWeapon(equipment);
  } else if (equipment.weapon1Id == itemId) {
    equipment.weapon1Id.clear();
  }
}

model::EquipItemResult equipWeapon(model::CharacterPlayerEquipment& equipment,
                                   const bmin::String& itemId,
                                   model::ItemType itemType) {
  const bool isTwoHanded = model::itemTypeIsTwoHandedWeapon(itemType);
  if (isTwoHanded) {
    if (!equipment.weapon1Id.empty()) {
      return model::EquipItemResult::SLOT_OCCUPIED;
    }
    if (equipment.weapon0Id.empty()) {
      equipment.weapon0Id = itemId;
      return model::EquipItemResult::EQUIPPED;
    }
    return model::EquipItemResult::TWO_HANDED_OFF_HAND;
  }
  if (equipment.weapon0Id.empty()) {
    equipment.weapon0Id = itemId;
    return model::EquipItemResult::EQUIPPED;
  }
  if (equipment.weapon1Id.empty()) {
    equipment.weapon1Id = itemId;
    return model::EquipItemResult::EQUIPPED;
  }
  return model::EquipItemResult::SLOT_OCCUPIED;
}

model::EquipItemResult equipSingleSlot(model::CharacterPlayerEquipment& equipment,
                                       const bmin::String& itemId,
                                       model::ItemType itemType) {
  auto* slot = equipmentSlotForItemType(equipment, itemType);
  if (slot == nullptr) {
    return model::EquipItemResult::NOT_EQUIPPABLE;
  }
  if (slot->empty()) {
    *slot = itemId;
    return model::EquipItemResult::EQUIPPED;
  }
  return *slot == itemId ? model::EquipItemResult::EQUIPPED
                         : model::EquipItemResult::SLOT_OCCUPIED;
}

void unequipItemById(model::CharacterPlayer& character,
                     const bmin::String& itemId,
                     model::ItemType itemType) {
  if (model::itemTypeUsesWeaponSlots(itemType)) {
    unequipWeaponByItemId(character.equipment, itemId);
    return;
  }
  auto* slot = equipmentSlotForItemType(character.equipment, itemType);
  if (slot != nullptr && *slot == itemId) {
    slot->clear();
  }
}

} // namespace

model::EquipItemResult toggleEquippedInventoryItem(
    model::CharacterPlayer& character,
    const bmin::String& itemId,
    const db::Database& database) {
  const auto inventoryItem = findInventoryItemById(character, itemId);
  if (!inventoryItem.has_value()) {
    return model::EquipItemResult::ITEM_NOT_IN_INVENTORY;
  }
  const auto& itemTemplate =
      database.getItemTemplate(bmin::toStringView(inventoryItem->itemName));
  if (!model::itemTypeIsEquippable(itemTemplate.itemType)) {
    return model::EquipItemResult::NOT_EQUIPPABLE;
  }
  if (model::characterPlayerIsItemEquippedById(character, itemId)) {
    unequipItemById(character, itemId, itemTemplate.itemType);
    return model::EquipItemResult::UNEQUIPPED;
  }
  if (model::itemTypeUsesWeaponSlots(itemTemplate.itemType)) {
    return equipWeapon(character.equipment, itemId, itemTemplate.itemType);
  }
  return equipSingleSlot(character.equipment, itemId, itemTemplate.itemType);
}

int inventoryWeight(const model::CharacterPlayer& character,
                    const db::Database& database) {
  int weight = 0;
  for (const auto& item : character.inventory) {
    const auto& itemTemplate =
        database.getItemTemplate(bmin::toStringView(item.itemName));
    weight += item.quantity * itemTemplate.weight;
  }
  return weight;
}

model::GiveItemResult giveInventoryItem(model::CharacterPlayer& from,
                                        model::CharacterPlayer& to,
                                        const bmin::String& itemId,
                                        int quantity,
                                        const db::Database& database) {
  const auto inventoryItem = findInventoryItemById(from, itemId);
  if (!inventoryItem.has_value()) {
    return model::GiveItemResult::ITEM_NOT_FOUND;
  }
  if (quantity < 1 || quantity > inventoryItem->quantity) {
    return model::GiveItemResult::INVALID_QUANTITY;
  }
  const auto& itemTemplate =
      database.getItemTemplate(bmin::toStringView(inventoryItem->itemName));
  if (inventoryWeight(to, database) + quantity * itemTemplate.weight >
      model::characterGetWeightCapacity(to)) {
    return model::GiveItemResult::TOO_HEAVY;
  }
  if (model::characterPlayerIsItemEquippedById(from, itemId) &&
      quantity == inventoryItem->quantity) {
    unequipItemById(from, itemId, itemTemplate.itemType);
  }
  model::characterPlayerRemoveItemFromInventoryById(from, itemId, quantity);
  model::characterPlayerAddItemToInventory(to, itemTemplate, quantity);
  return model::GiveItemResult::SUCCESS;
}

} // namespace game
