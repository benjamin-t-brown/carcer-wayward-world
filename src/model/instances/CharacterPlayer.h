#pragma once

#include "model/instances/ItemInstance.h"
#include "model/stats/CharacterStats.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/RuneTypes.h"
#include "model/templates/UtilityTypes.h"
#include <optional>

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

  CharacterPlayer(const CharacterTemplate& params = CharacterTemplate(),
                  const bmin::DynArray<CharacterInventoryItem>& inventory = {},
                  const CharacterPlayerEquipment& equipment = {});
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

bool characterPlayerReorderInventoryItem(CharacterPlayer& characterPlayer,
                                         size_t index,
                                         int direction);
int characterGetWeightCapacity(const CharacterPlayer& characterPlayer);

/** Copy starting known/ready spell lists from template onto a party member. */
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

} // namespace model
