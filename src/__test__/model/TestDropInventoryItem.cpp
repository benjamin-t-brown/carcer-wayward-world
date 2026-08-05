#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/actions/ui/UiDropInventoryItem.hpp"
#include "bmin/String.h"
#include <cassert>

#define TEST_NAME "TestDropInventoryItem"

namespace {

void addItemTemplate(db::Database& database, const bmin::String& name) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = name;
  itemTemplate.weight = 1;
  database.addItemTemplate(itemTemplate);
}

model::CharacterInventoryItem makeInventoryItem(const bmin::String& id,
                                                const bmin::String& itemName,
                                                int quantity = 1) {
  return {.itemName = itemName, .id = id, .quantity = quantity};
}

model::CharacterInstance makeMapCharacter(const bmin::String& id, int x, int y) {
  model::CharacterInstance character;
  character.id = id;
  character.x = x;
  character.y = y;
  return character;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  db::Database database;
  addItemTemplate(database, "PotionHealing");
  state::DatabaseInterface::setDatabase(&database);

  {
    state::State state;
    model::CharacterPlayer leader;
    leader.instanceId = "leader-id";
    leader.inventory = {makeInventoryItem("potion-1", "PotionHealing", 2)};
    model::CharacterPlayer companion;
    companion.instanceId = "companion-id";
    companion.inventory = {makeInventoryItem("potion-2", "PotionHealing", 1)};
    state.player.party = {leader, companion};
    state.world.activeMap.characters = {makeMapCharacter("leader-id", 3, 4)};

    state::actions::UiDropInventoryItem("companion-id", "potion-2").execute(&state);

    assert(state.player.party[1].inventory.empty());
    assert(state.world.activeMap.items.size() == 1);
    assert(state.world.activeMap.items[0].itemTemplateName == "PotionHealing");
    assert(state.world.activeMap.items[0].quantity == 1);
    assert(state.world.activeMap.items[0].x == 3);
    assert(state.world.activeMap.items[0].y == 4);
  }

  {
    state::State state;
    model::CharacterPlayer leader;
    leader.instanceId = "leader-id";
    leader.inventory = {makeInventoryItem("potion-1", "PotionHealing", 1)};
    model::CharacterPlayer companion;
    companion.instanceId = "companion-id";
    companion.inventory = {makeInventoryItem("potion-2", "PotionHealing", 1)};
    state.player.party = {leader, companion};
    state.world.activeMap.characters = {
        makeMapCharacter("leader-id", 1, 1),
        makeMapCharacter("companion-id", 5, 6),
    };

    state::actions::UiDropInventoryItem("companion-id", "potion-2").execute(&state);

    assert(state.player.party[1].inventory.empty());
    assert(state.world.activeMap.items.size() == 1);
    assert(state.world.activeMap.items[0].x == 5);
    assert(state.world.activeMap.items[0].y == 6);
  }

  state::DatabaseInterface::setDatabase(nullptr);

  LOG(INFO) << "Finished " << TEST_NAME << LOG_ENDL;
  return 0;
}
