#include "model/instances/CharacterPlayer.h"
#include "model/templates/UtilityTypes.h"
#include <utility>

namespace model {

CharacterPlayer::CharacterPlayer(
    const CharacterTemplate& characterTemplate,
    const bmin::DynArray<CharacterInventoryItem>& characterInventory,
    const CharacterPlayerEquipment& characterEquipment) {
  instanceId = createRandomId();
  params = characterTemplate;
  initCharacterStatsFromTemplate(stats, characterTemplate);
  currentHp = characterTemplate.combat.hp;
  currentMp = characterTemplate.combat.mp;
  inventory = characterInventory;
  equipment = characterEquipment;
  applyCharacterTemplateStartingSpells(*this, characterTemplate);
}


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


int characterGetWeightCapacity(const CharacterPlayer& characterPlayer) {
  return 100; // TODO derive
}

} // namespace model
