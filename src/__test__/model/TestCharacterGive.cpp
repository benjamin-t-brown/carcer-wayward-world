#include "db/Database.h"
#include "lib/sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/Items.h"
#include "bmin/String.h"
#include <cassert>

#define TEST_NAME "TestCharacterGive"

namespace {

void addItemTemplate(db::Database& database,
                     const bmin::String& name,
                     int weight,
                     bool stackable = false) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = name;
  itemTemplate.weight = weight;
  itemTemplate.stackable = stackable;
  database.addItemTemplate(itemTemplate);
}

model::CharacterInventoryItem makeInventoryItem(const bmin::String& id,
                                                const bmin::String& itemName,
                                                int quantity = 1) {
  return {.itemName = itemName, .id = id, .quantity = quantity};
}

void assertResult(model::GiveItemResult actual, model::GiveItemResult expected) {
  assert(static_cast<int>(actual) == static_cast<int>(expected));
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  db::Database database;
  addItemTemplate(database, "LightItem", 5);
  addItemTemplate(database, "HeavyItem", 50);
  addItemTemplate(database, "StackItem", 2, true);

  model::CharacterPlayer giver;
  model::CharacterPlayer recipient;
  giver.inventory = {makeInventoryItem("item1", "LightItem", 3)};
  recipient.inventory = {};

  assertResult(model::characterPlayerGiveInventoryItem(
                   giver, recipient, "item1", 2, database),
               model::GiveItemResult::SUCCESS);
  assert(giver.inventory.size() == 1);
  assert(giver.inventory[0].quantity == 1);
  assert(recipient.inventory.size() == 1);
  assert(recipient.inventory[0].quantity == 2);
  assert(recipient.inventory[0].itemName == "LightItem");

  giver.inventory = {makeInventoryItem("heavy1", "HeavyItem", 1)};
  recipient.inventory = {makeInventoryItem("fill", "LightItem", 18)};
  assertResult(model::characterPlayerGiveInventoryItem(
                   giver, recipient, "heavy1", 1, database),
               model::GiveItemResult::TOO_HEAVY);

  assertResult(
      model::characterPlayerGiveInventoryItem(giver, recipient, "missing", 1, database),
      model::GiveItemResult::ITEM_NOT_FOUND);

  assertResult(model::characterPlayerGiveInventoryItem(
                   giver, recipient, "heavy1", 0, database),
               model::GiveItemResult::INVALID_QUANTITY);

  LOG(INFO) << "Finished " << TEST_NAME << LOG_ENDL;
  return 0;
}
