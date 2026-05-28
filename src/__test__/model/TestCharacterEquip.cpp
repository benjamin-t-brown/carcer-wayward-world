#include "db/Database.h"
#include "lib/sdl2w/Logger.h"
#include "model/Character.h"
#include "model/Items.h"
#include <cassert>
#include <string>

#define TEST_NAME "TestCharacterEquip"

namespace {

void addItemTemplate(db::Database& database,
                     const std::string& name,
                     model::ItemType itemType) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = name;
  itemTemplate.itemType = itemType;
  database.addItemTemplate(itemTemplate);
}

model::CharacterInventoryItem makeInventoryItem(const std::string& id,
                                                const std::string& itemName) {
  return {.itemName = itemName, .id = id, .quantity = 1};
}

void assertResult(model::EquipItemResult actual, model::EquipItemResult expected) {
  assert(static_cast<int>(actual) == static_cast<int>(expected));
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  db::Database database;
  addItemTemplate(database, "DaggerOne", model::ItemType::WEAPON_MELEE);
  addItemTemplate(database, "SwordTwoHand", model::ItemType::WEAPON_MELEE_2H);
  addItemTemplate(database, "HatA", model::ItemType::HAT);
  addItemTemplate(database, "HatB", model::ItemType::HAT);

  model::CharacterPlayer character;
  character.inventory = {
      makeInventoryItem("dagger1", "DaggerOne"),
      makeInventoryItem("dagger2", "DaggerOne"),
      makeInventoryItem("dagger3", "DaggerOne"),
      makeInventoryItem("sword2h", "SwordTwoHand"),
      makeInventoryItem("hatA", "HatA"),
      makeInventoryItem("hatB", "HatB"),
  };

  assertResult(model::characterPlayerToggleEquipItem(character, "dagger1", database),
               model::EquipItemResult::EQUIPPED);
  assert(character.equipment.weapon0Id == "dagger1");
  assert(character.equipment.weapon1Id.empty());

  assertResult(model::characterPlayerToggleEquipItem(character, "dagger2", database),
               model::EquipItemResult::EQUIPPED);
  assert(character.equipment.weapon0Id == "dagger1");
  assert(character.equipment.weapon1Id == "dagger2");

  assertResult(model::characterPlayerToggleEquipItem(character, "dagger3", database),
               model::EquipItemResult::SLOT_OCCUPIED);
  assert(character.equipment.weapon1Id == "dagger2");

  assertResult(model::characterPlayerToggleEquipItem(character, "dagger1", database),
               model::EquipItemResult::UNEQUIPPED);
  assert(character.equipment.weapon0Id == "dagger2");
  assert(character.equipment.weapon1Id.empty());

  character.equipment = {};
  assertResult(model::characterPlayerToggleEquipItem(character, "hatA", database),
               model::EquipItemResult::EQUIPPED);
  assert(character.equipment.hatId == "hatA");

  assertResult(model::characterPlayerToggleEquipItem(character, "hatB", database),
               model::EquipItemResult::SLOT_OCCUPIED);
  assert(character.equipment.hatId == "hatA");

  assertResult(model::characterPlayerToggleEquipItem(character, "hatA", database),
               model::EquipItemResult::UNEQUIPPED);
  assert(character.equipment.hatId.empty());

  character.equipment = {};
  character.inventory.push_back(makeInventoryItem("daggerOff", "DaggerOne"));
  assertResult(model::characterPlayerToggleEquipItem(character, "daggerOff", database),
               model::EquipItemResult::EQUIPPED);
  assert(character.equipment.weapon0Id == "daggerOff");

  assertResult(model::characterPlayerToggleEquipItem(character, "sword2h", database),
               model::EquipItemResult::TWO_HANDED_OFF_HAND);
  assert(character.equipment.weapon0Id == "daggerOff");

  character.equipment.weapon0Id.clear();
  character.equipment.weapon1Id = "daggerOff";
  assertResult(model::characterPlayerToggleEquipItem(character, "sword2h", database),
               model::EquipItemResult::SLOT_OCCUPIED);
  assert(character.equipment.weapon1Id == "daggerOff");

  character.equipment = {};
  assertResult(model::characterPlayerToggleEquipItem(character, "sword2h", database),
               model::EquipItemResult::EQUIPPED);
  assert(character.equipment.weapon0Id == "sword2h");

  LOG(INFO) << "Finished " << TEST_NAME << LOG_ENDL;
  return 0;
}
