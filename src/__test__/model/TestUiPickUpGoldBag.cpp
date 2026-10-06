#include "actions/navigation/UiPickUpItem.hpp"
#include "db/Database.h"
#include "game/combat/DropTables.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"

namespace {

void addItemTemplate(db::Database& database, const bmin::String& name, int weight = 0) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = name;
  itemTemplate.weight = weight;
  database.addItemTemplate(itemTemplate);
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestUiPickUpGoldBag" << LOG_ENDL;

  db::Database database;
  addItemTemplate(database, "GoldBag", 0);
  addItemTemplate(database, "FoodBag", 0);
  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  model::CharacterPlayer leader;
  leader.instanceId = "leader-id";
  state.player.party = {leader};
  state.player.gold = 10;
  state.player.food = 5;

  state.world.activeMap.items.pushBack(model::ItemInstance{
      .id = "gold-bag-1",
      .itemTemplateName = "GoldBag",
      .quantity = 1,
      .amount = 25,
  });
  state.world.activeMap.items.pushBack(model::ItemInstance{
      .id = "food-bag-1",
      .itemTemplateName = "FoodBag",
      .quantity = 1,
      .amount = 3,
  });

  state::actions::UiPickUpItem("gold-bag-1").execute(&state);
  if (state.player.gold != 35) {
    LOG(ERROR) << "Expected player gold 35, got " << state.player.gold << LOG_ENDL;
    return 1;
  }
  if (state.player.party[0].inventory.size() != 0) {
    LOG(ERROR) << "Gold bag should not enter inventory" << LOG_ENDL;
    return 1;
  }
  if (state.world.activeMap.items.size() != 1) {
    LOG(ERROR) << "Expected one map item after gold pickup" << LOG_ENDL;
    return 1;
  }

  state::actions::UiPickUpItem("food-bag-1").execute(&state);
  if (state.player.food != 8) {
    LOG(ERROR) << "Expected player food 8, got " << state.player.food << LOG_ENDL;
    return 1;
  }
  if (state.world.activeMap.items.size() != 0) {
    LOG(ERROR) << "Expected no map items after food pickup" << LOG_ENDL;
    return 1;
  }

  state::DatabaseInterface::setDatabase(nullptr);

  LOG(INFO) << "TestUiPickUpGoldBag completed successfully" << LOG_ENDL;
  return 0;
}
