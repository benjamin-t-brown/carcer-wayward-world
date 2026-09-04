module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#include <algorithm>

export module carcer.model.instances.CharacterPlayer;
export import bmin.containers;
import bmin.string_interop;
export import carcer.db;
export import carcer.model.instances.ItemInstance;
export import carcer.model.stats.CharacterStats;
export import carcer.model.templates.CharacterTemplate;
export import carcer.model.templates.Items;
export import carcer.model.templates.RuneTypes;
export import carcer.model.templates.UtilityTypes;
import sdl2w;
#include "macros.h"

export {

// --- from model/instances/CharacterPlayer.h ---
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

/** Owned rune tally on the character (not bag inventory). */
struct CharacterAvailableRune {
  RuneType type = RuneType::HEAT;
  int count = 0;
};

struct CharacterPlayer;
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

struct CharacterPlayer {
  static constexpr size_t kRuneSlotCount = 8;

  bmin::String instanceId;
  bmin::String name;
  bmin::String templateName;
  bmin::DynArray<CharacterInventoryItem> inventory;
  CharacterPlayerEquipment equipment;
  CharacterTemplate params;
  CharacterStats stats;
  int currentHp = 0;
  int currentMp = 0;
  bmin::DynArray<bmin::String> knownSpells;
  bmin::DynArray<bmin::String> readySpells;
  // Tallies of runes the character owns; equip capacity comes from here.
  bmin::DynArray<CharacterAvailableRune> availableRunes;
  // Dense list of equipped rune types; size 0..kRuneSlotCount, no mid-list holes.
  bmin::DynArray<RuneType> equippedRunes;

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
    applyCharacterTemplateStartingSpells(*this, _params);
  }
};

bmin::String characterPlayerGetSprite(const CharacterPlayer& characterPlayer);
bmin::String characterPlayerGetSpriteAtIndexOffset(const CharacterPlayer& characterPlayer,
                                                   int indexOffset);

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

enum class EquipRuneResult {
  EQUIPPED,
  UNEQUIPPED,
  NOT_A_RUNE,
  ITEM_NOT_IN_INVENTORY,
  SLOT_OCCUPIED,
  SLOT_EMPTY,
  INVALID_SLOT,
  ALREADY_EQUIPPED,
  NO_RUNE_AVAILABLE,
};

int characterPlayerCountAvailableRunesOfType(const CharacterPlayer& characterPlayer,
                                            RuneType runeType);
void characterPlayerSetAvailableRuneCount(CharacterPlayer& characterPlayer,
                                          RuneType runeType,
                                          int count);
int characterPlayerCountEquippedRunesOfType(const CharacterPlayer& characterPlayer,
                                            RuneType runeType);
bool characterPlayerCanEquipRuneType(const CharacterPlayer& characterPlayer,
                                     RuneType runeType);
std::optional<RuneType>
characterPlayerFindFirstEquippableRuneType(const CharacterPlayer& characterPlayer);
/** Insert `runeType` (sorted by type) when list has room and capacity remains. */
EquipRuneResult characterPlayerEquipRuneType(CharacterPlayer& characterPlayer,
                                             RuneType runeType);
/** Erase at `slotIndex` and compact; fails if index is empty / out of range. */
EquipRuneResult characterPlayerUnequipRuneFromSlot(CharacterPlayer& characterPlayer,
                                                   size_t slotIndex);
/** Unequip the last equipped occurrence of `runeType`, if any. */
EquipRuneResult characterPlayerUnequipOneRuneOfType(CharacterPlayer& characterPlayer,
                                                     RuneType runeType);
/**
 * Filled slot (`slotIndex < size`): unequip and compact.
 * First empty (`slotIndex == size`): append first equippable available rune type.
 * `slotIndex > size` or past max: INVALID_SLOT.
 */
EquipRuneResult characterPlayerToggleManaSlotRune(CharacterPlayer& characterPlayer,
                                                   size_t slotIndex);

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

/** Copy starting known/ready spell lists from template onto a party member. */
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

} // namespace model

} // export

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

bmin::String characterPlayerGetSpriteAtIndexOffset(const CharacterPlayer& characterPlayer,
                                                     int indexOffset) {
  return characterGetSpriteAtIndexOffset(characterPlayer.params, indexOffset);
}

bmin::String characterPlayerGetSprite(const CharacterPlayer& characterPlayer) {
  return characterPlayerGetSpriteAtIndexOffset(characterPlayer, 0);
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

int characterPlayerCountAvailableRunesOfType(const CharacterPlayer& characterPlayer,
                                            RuneType runeType) {
  for (const auto& entry : characterPlayer.availableRunes) {
    if (entry.type == runeType) {
      return entry.count;
    }
  }
  return 0;
}

void characterPlayerSetAvailableRuneCount(CharacterPlayer& characterPlayer,
                                          RuneType runeType,
                                          int count) {
  const int clamped = count < 0 ? 0 : count;
  for (size_t i = 0; i < characterPlayer.availableRunes.size(); ++i) {
    if (characterPlayer.availableRunes[i].type != runeType) {
      continue;
    }
    if (clamped == 0) {
      characterPlayer.availableRunes.erase(i);
    } else {
      characterPlayer.availableRunes[i].count = clamped;
    }
    return;
  }
  if (clamped > 0) {
    characterPlayer.availableRunes.pushBack(
        CharacterAvailableRune{.type = runeType, .count = clamped});
  }
}

int characterPlayerCountEquippedRunesOfType(const CharacterPlayer& characterPlayer,
                                            RuneType runeType) {
  auto count = int{0};
  for (const auto& equipped : characterPlayer.equippedRunes) {
    if (equipped == runeType) {
      count++;
    }
  }
  return count;
}

bool characterPlayerCanEquipRuneType(const CharacterPlayer& characterPlayer,
                                     RuneType runeType) {
  if (characterPlayer.equippedRunes.size() >= CharacterPlayer::kRuneSlotCount) {
    return false;
  }
  return characterPlayerCountAvailableRunesOfType(characterPlayer, runeType) >
         characterPlayerCountEquippedRunesOfType(characterPlayer, runeType);
}

std::optional<RuneType>
characterPlayerFindFirstEquippableRuneType(const CharacterPlayer& characterPlayer) {
  for (int i = 0; i < kRuneTypeCount; ++i) {
    const auto runeType = runeTypeFromIndex(i);
    if (characterPlayerCanEquipRuneType(characterPlayer, runeType)) {
      return runeType;
    }
  }
  return std::nullopt;
}

EquipRuneResult characterPlayerEquipRuneType(CharacterPlayer& characterPlayer,
                                             RuneType runeType) {
  if (characterPlayer.equippedRunes.size() >= CharacterPlayer::kRuneSlotCount) {
    return EquipRuneResult::INVALID_SLOT;
  }
  if (!characterPlayerCanEquipRuneType(characterPlayer, runeType)) {
    return EquipRuneResult::NO_RUNE_AVAILABLE;
  }
  // Keep same types adjacent (enum order: HEAT, ENTROPY, REGROWTH, ...).
  size_t insertAt = characterPlayer.equippedRunes.size();
  const int newIndex = runeTypeIndex(runeType);
  for (size_t i = 0; i < characterPlayer.equippedRunes.size(); ++i) {
    if (runeTypeIndex(characterPlayer.equippedRunes[i]) > newIndex) {
      insertAt = i;
      break;
    }
  }
  characterPlayer.equippedRunes.insert(
      characterPlayer.equippedRunes.begin() + insertAt, runeType);
  return EquipRuneResult::EQUIPPED;
}

EquipRuneResult characterPlayerUnequipRuneFromSlot(CharacterPlayer& characterPlayer,
                                                   size_t slotIndex) {
  if (slotIndex >= CharacterPlayer::kRuneSlotCount) {
    return EquipRuneResult::INVALID_SLOT;
  }
  if (slotIndex >= characterPlayer.equippedRunes.size()) {
    return EquipRuneResult::SLOT_EMPTY;
  }
  characterPlayer.equippedRunes.erase(slotIndex);
  return EquipRuneResult::UNEQUIPPED;
}

EquipRuneResult characterPlayerUnequipOneRuneOfType(CharacterPlayer& characterPlayer,
                                                     RuneType runeType) {
  for (int i = static_cast<int>(characterPlayer.equippedRunes.size()) - 1; i >= 0;
       --i) {
    if (characterPlayer.equippedRunes[static_cast<size_t>(i)] == runeType) {
      return characterPlayerUnequipRuneFromSlot(characterPlayer,
                                               static_cast<size_t>(i));
    }
  }
  return EquipRuneResult::SLOT_EMPTY;
}

EquipRuneResult characterPlayerToggleManaSlotRune(CharacterPlayer& characterPlayer,
                                                   size_t slotIndex) {
  if (slotIndex >= CharacterPlayer::kRuneSlotCount) {
    return EquipRuneResult::INVALID_SLOT;
  }

  if (slotIndex < characterPlayer.equippedRunes.size()) {
    return characterPlayerUnequipRuneFromSlot(characterPlayer, slotIndex);
  }

  if (slotIndex > characterPlayer.equippedRunes.size()) {
    return EquipRuneResult::INVALID_SLOT;
  }

  const auto runeType = characterPlayerFindFirstEquippableRuneType(characterPlayer);
  if (!runeType.has_value()) {
    return EquipRuneResult::NO_RUNE_AVAILABLE;
  }
  return characterPlayerEquipRuneType(characterPlayer, *runeType);
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

void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate) {
  character.knownSpells.clear();
  for (const auto& spellName : characterTemplate.startingKnownSpells) {
    if (spellName.empty()) {
      continue;
    }
    bool alreadyKnown = false;
    for (const auto& known : character.knownSpells) {
      if (known == spellName) {
        alreadyKnown = true;
        break;
      }
    }
    if (!alreadyKnown) {
      character.knownSpells.pushBack(spellName);
    }
  }

  character.readySpells.clear();
  for (const auto& spellName : characterTemplate.startingReadySpells) {
    if (spellName.empty()) {
      continue;
    }
    bool isKnown = false;
    for (const auto& known : character.knownSpells) {
      if (known == spellName) {
        isKnown = true;
        break;
      }
    }
    if (!isKnown) {
      continue;
    }
    bool alreadyReady = false;
    for (const auto& ready : character.readySpells) {
      if (ready == spellName) {
        alreadyReady = true;
        break;
      }
    }
    if (!alreadyReady) {
      character.readySpells.pushBack(spellName);
    }
  }
}

} // namespace model
