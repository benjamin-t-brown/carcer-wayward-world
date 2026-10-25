#include "model/templates/CharacterTemplate.h"
#include "bmin/StringInterop.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/UtilityTypes.h"
#include <charconv>
#include <stdexcept>
#include <system_error>

namespace model {

CharacterSpriteLayout characterSpriteLayoutFromString(const bmin::String& value) {
  if (value == "weaponCombos") {
    return CharacterSpriteLayout::WeaponCombos;
  }
  return CharacterSpriteLayout::Default;
}

CombatSpellAiSelection combatSpellAiSelectionFromString(const bmin::String& value) {
  if (value == "RANDOM") {
    return CombatSpellAiSelection::RANDOM;
  }
  if (value == "FIRST" || value.empty()) {
    return CombatSpellAiSelection::FIRST;
  }
  throw std::runtime_error(
      (bmin::String("Invalid combatSpellAi selection: ") + value).cStr());
}

bmin::String getStringFromCombatSpellAiSelection(CombatSpellAiSelection selection) {
  switch (selection) {
  case CombatSpellAiSelection::RANDOM:
    return "RANDOM";
  case CombatSpellAiSelection::FIRST:
    break;
  }
  return "FIRST";
}

ShootTargeting shootTargetingFromString(const bmin::String& value) {
  if (value == "LOWEST_HEALTH") {
    return ShootTargeting::LOWEST_HEALTH;
  }
  if (value == "ONLY_PC") {
    return ShootTargeting::ONLY_PC;
  }
  if (value == "RANDOM" || value.empty()) {
    return ShootTargeting::RANDOM;
  }
  throw std::runtime_error((bmin::String("Invalid shoot targeting: ") + value).cStr());
}

bmin::String getStringFromShootTargeting(ShootTargeting targeting) {
  switch (targeting) {
  case ShootTargeting::LOWEST_HEALTH:
    return "LOWEST_HEALTH";
  case ShootTargeting::ONLY_PC:
    return "ONLY_PC";
  case ShootTargeting::RANDOM:
    break;
  }
  return "RANDOM";
}

CharacterStartingEquipSlot characterStartingEquipSlotFromString(const bmin::String& value) {
  if (value == "WEAPON0") {
    return CharacterStartingEquipSlot::WEAPON0;
  }
  if (value == "WEAPON1") {
    return CharacterStartingEquipSlot::WEAPON1;
  }
  if (value == "AMMO") {
    return CharacterStartingEquipSlot::AMMO;
  }
  if (value == "NONE" || value.empty()) {
    return CharacterStartingEquipSlot::NONE;
  }
  throw std::runtime_error(
      (bmin::String("Invalid starting equip slot: ") + value).cStr());
}

bmin::String getStringFromCharacterStartingEquipSlot(CharacterStartingEquipSlot slot) {
  switch (slot) {
  case CharacterStartingEquipSlot::WEAPON0:
    return "WEAPON0";
  case CharacterStartingEquipSlot::WEAPON1:
    return "WEAPON1";
  case CharacterStartingEquipSlot::AMMO:
    return "AMMO";
  case CharacterStartingEquipSlot::NONE:
    break;
  }
  return "NONE";
}

CombatSpellAiZoneTargeting combatSpellAiZoneTargetingFromString(const bmin::String& value) {
  if (value == "RANDOM_IN_RANGE") {
    return CombatSpellAiZoneTargeting::RANDOM_IN_RANGE;
  }
  if (value == "ONLY_PC") {
    return CombatSpellAiZoneTargeting::ONLY_PC;
  }
  if (value == "DEFAULT" || value.empty()) {
    return CombatSpellAiZoneTargeting::DEFAULT;
  }
  throw std::runtime_error(
      (bmin::String("Invalid combatSpellAi zone targeting: ") + value).cStr());
}

bmin::String getStringFromCombatSpellAiZoneTargeting(CombatSpellAiZoneTargeting targeting) {
  switch (targeting) {
  case CombatSpellAiZoneTargeting::RANDOM_IN_RANGE:
    return "RANDOM_IN_RANGE";
  case CombatSpellAiZoneTargeting::ONLY_PC:
    return "ONLY_PC";
  case CombatSpellAiZoneTargeting::DEFAULT:
    break;
  }
  return "DEFAULT";
}

CombatSpellAiConditionType combatSpellAiConditionTypeFromString(const bmin::String& value) {
  if (value == "ALWAYS") {
    return CombatSpellAiConditionType::ALWAYS;
  }
  if (value == "SELF_HP_BELOW_PERCENT") {
    return CombatSpellAiConditionType::SELF_HP_BELOW_PERCENT;
  }
  if (value == "SELF_HP_ABOVE_PERCENT") {
    return CombatSpellAiConditionType::SELF_HP_ABOVE_PERCENT;
  }
  if (value == "ZONE_HOSTILE_COUNT_AT_LEAST") {
    return CombatSpellAiConditionType::ZONE_HOSTILE_COUNT_AT_LEAST;
  }
  if (value == "HAS_STATUS") {
    return CombatSpellAiConditionType::HAS_STATUS;
  }
  if (value == "LACKS_STATUS") {
    return CombatSpellAiConditionType::LACKS_STATUS;
  }
  if (value == "OWN_TURN_AT_MOST") {
    return CombatSpellAiConditionType::OWN_TURN_AT_MOST;
  }
  throw std::runtime_error(
      (bmin::String("Invalid combatSpellAi condition type: ") + value).cStr());
}

bmin::String getStringFromCombatSpellAiConditionType(CombatSpellAiConditionType type) {
  switch (type) {
  case CombatSpellAiConditionType::SELF_HP_BELOW_PERCENT:
    return "SELF_HP_BELOW_PERCENT";
  case CombatSpellAiConditionType::SELF_HP_ABOVE_PERCENT:
    return "SELF_HP_ABOVE_PERCENT";
  case CombatSpellAiConditionType::ZONE_HOSTILE_COUNT_AT_LEAST:
    return "ZONE_HOSTILE_COUNT_AT_LEAST";
  case CombatSpellAiConditionType::HAS_STATUS:
    return "HAS_STATUS";
  case CombatSpellAiConditionType::LACKS_STATUS:
    return "LACKS_STATUS";
  case CombatSpellAiConditionType::OWN_TURN_AT_MOST:
    return "OWN_TURN_AT_MOST";
  case CombatSpellAiConditionType::ALWAYS:
    break;
  }
  return "ALWAYS";
}

bmin::String getStringFromCharacterSpriteLayout(CharacterSpriteLayout layout) {
  switch (layout) {
  case CharacterSpriteLayout::WeaponCombos:
    return "weaponCombos";
  case CharacterSpriteLayout::Default:
    break;
  }
  return "default";
}

namespace {

int parseSpriteOffsetIndex(const bmin::String& spriteOffset) {
  const auto view = bmin::toStringView(spriteOffset);
  int value = 0;
  const auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value);
  if (ec != std::errc{} || ptr != view.data() + view.size()) {
    throw std::runtime_error("Invalid character spriteOffset");
  }
  return value;
}

bmin::String buildCharacterSpriteName(const bmin::String& spritesheetName,
                                      const bmin::String& spriteOffset,
                                      int indexOffset) {
  const auto index = parseSpriteOffsetIndex(spriteOffset) + indexOffset;
  return spritesheetName + "_" + bmin::toString(index);
}

} // namespace

bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                               int indexOffset) {
  return buildCharacterSpriteName(
      characterTemplate.spritesheetName, characterTemplate.spriteOffset, indexOffset);
}

bmin::String characterGetSprite(const CharacterTemplate& characterTemplate) {
  return characterGetSpriteAtIndexOffset(characterTemplate, 0);
}

namespace {

void seedStartingInventory(CharacterInstance& character,
                           const CharacterTemplate& characterTemplate) {
  if (character.inventorySeeded) {
    return;
  }
  character.inventorySeeded = true;
  for (size_t i = 0; i < characterTemplate.startingInventory.size(); ++i) {
    const auto& starting = characterTemplate.startingInventory[i];
    if (starting.itemName.empty()) {
      continue;
    }
    CharacterInventoryItem item;
    item.itemName = starting.itemName;
    item.id = createRandomId();
    item.quantity = starting.quantity < 1 ? 1 : starting.quantity;
    const auto itemId = item.id;
    character.inventory.pushBack(item);
    switch (starting.equipSlot) {
    case CharacterStartingEquipSlot::WEAPON0:
      character.weapon0Id = itemId;
      break;
    case CharacterStartingEquipSlot::WEAPON1:
      character.weapon1Id = itemId;
      break;
    case CharacterStartingEquipSlot::AMMO:
      character.ammoId = itemId;
      break;
    case CharacterStartingEquipSlot::NONE:
      break;
    }
  }
}

} // namespace

void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate) {
  character.type = characterTemplate.type;
  character.shootTargeting = characterTemplate.shootTargeting;
  seedStartingInventory(character, characterTemplate);
  character.label = characterTemplate.label;
  character.behaviorName = characterTemplate.behavior.behaviorName;
  character.visionRadius = characterTemplate.vision.radius;
  character.combatBehaviorTown = characterTemplate.combatBehavior.town;
  character.combatBehaviorCombat = characterTemplate.combatBehavior.combat;
  character.combatSpellAi = characterTemplate.combatSpellAi;
  character.maxHp = characterTemplate.combat.hp;
  character.maxMp = characterTemplate.combat.mp;
  initCharacterStatsFromTemplate(character.stats, characterTemplate);
  if (character.name.empty()) {
    character.name = characterTemplate.label.empty() ? characterTemplate.name
                                                     : characterTemplate.label;
  }
  if (character.templateName.empty()) {
    character.templateName = characterTemplate.name;
  }
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
