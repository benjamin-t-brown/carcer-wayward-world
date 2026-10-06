#include "actions/combat/PerformCharacterDefeated.hpp"
#include "db/Database.h"
#include "game/combat/DropTables.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/DropTables.hpp"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"

namespace {

model::CharacterInstance makeEnemy(int x, int y) {
  model::CharacterInstance enemy;
  enemy.id = "enemy-1";
  enemy.templateName = "lootEnemy";
  enemy.x = x;
  enemy.y = y;
  return enemy;
}

void addItem(db::Database& database, const bmin::String& name) {
  model::ItemTemplate item;
  item.name = name;
  item.weight = 1;
  database.addItemTemplate(item);
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestPerformCharacterDefeatedDrops" << LOG_ENDL;

  db::Database database;
  addItem(database, "PotionHealing");
  addItem(database, "GoldBag");
  addItem(database, "FoodBag");

  model::DropTableEntry entry;
  entry.item = "PotionHealing";
  model::DropTableTemplate table;
  table.name = "defeatTestTable";
  table.label = "Defeat Test";
  table.entries.pushBack(entry);
  database.addDropTable(table);

  model::DropTableEntry goldEntry;
  goldEntry.isGoldRangeEntry = true;
  goldEntry.goldMin = 12;
  goldEntry.goldMax = 12;
  model::DropTableTemplate goldTable;
  goldTable.name = "defeatGoldTable";
  goldTable.label = "Defeat Gold";
  goldTable.entries.pushBack(goldEntry);
  database.addDropTable(goldTable);

  auto enemyTemplate = model::CharacterTemplate{};
  enemyTemplate.name = "lootEnemy";
  enemyTemplate.combat.dropTables.pushBack("defeatTestTable");
  enemyTemplate.sound.deathSoundName = "yell_monster1";
  database.addCharacterTemplate(enemyTemplate);

  auto goldEnemyTemplate = model::CharacterTemplate{};
  goldEnemyTemplate.name = "goldLootEnemy";
  goldEnemyTemplate.combat.dropTables.pushBack("defeatGoldTable");
  goldEnemyTemplate.sound.deathSoundName = "yell_monster1";
  database.addCharacterTemplate(goldEnemyTemplate);

  state::DatabaseInterface::setDatabase(&database);

  {
    state::State state;
    state.world.activeMap.characters.pushBack(makeEnemy(4, 7));

    state::actions::PerformCharacterDefeated("enemy-1").execute(&state);

    if (state.world.activeMap.items.size() != 1) {
      LOG(ERROR) << "Expected 1 dropped item, got " << state.world.activeMap.items.size()
                 << LOG_ENDL;
      return 1;
    }
    const auto& dropped = state.world.activeMap.items[0];
    if (dropped.itemTemplateName != "PotionHealing" || dropped.quantity != 1 ||
        dropped.x != 4 || dropped.y != 7 || dropped.id.empty()) {
      LOG(ERROR) << "Dropped item fields mismatch" << LOG_ENDL;
      return 1;
    }
  }

  {
    state::State state;
    model::CharacterInstance goldEnemy = makeEnemy(2, 3);
    goldEnemy.templateName = "goldLootEnemy";
    state.world.activeMap.characters.pushBack(goldEnemy);

    state::actions::PerformCharacterDefeated("enemy-1").execute(&state);

    if (state.world.activeMap.items.size() != 1) {
      LOG(ERROR) << "Expected 1 gold bag drop, got " << state.world.activeMap.items.size()
                 << LOG_ENDL;
      return 1;
    }
    const auto& dropped = state.world.activeMap.items[0];
    if (dropped.itemTemplateName != "GoldBag" || dropped.amount != 12 || dropped.quantity != 1 ||
        dropped.x != 2 || dropped.y != 3) {
      LOG(ERROR) << "Gold bag drop fields mismatch" << LOG_ENDL;
      return 1;
    }
  }

  state::DatabaseInterface::setDatabase(nullptr);

  LOG(INFO) << "TestPerformCharacterDefeatedDrops completed successfully" << LOG_ENDL;
  return 0;
}
