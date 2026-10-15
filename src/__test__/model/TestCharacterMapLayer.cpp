#include "db/Database.h"
#include "game/map/ActiveMapCharacters.h"
#include "game/map/MapPathfinding.h"
#include "game/map/TileTriggers.h"
#include "model/instances/MapInstance.h"
#include "model/instances/World.hpp"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Maps.h"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

model::CarcerMapTemplate makeLayeredCharacterMap() {
  auto mapTemplate = model::CarcerMapTemplate{};
  mapTemplate.name = "layer_char_map";
  mapTemplate.label = "Layer Char Map";
  mapTemplate.width = 3;
  mapTemplate.height = 3;
  mapTemplate.spriteWidth = 28;
  mapTemplate.spriteHeight = 32;
  mapTemplate.tilesets.pushBack("test_terrain");

  auto makeLayer = []() {
    auto layer = bmin::DynArray<int>{};
    for (auto i = 0; i < 9; i++) {
      layer.pushBack(0);
      layer.pushBack(0);
    }
    return layer;
  };
  mapTemplate.tiles[-1] = makeLayer();
  mapTemplate.tiles[0] = makeLayer();

  auto basement = model::MapCharacterPlacement{};
  basement.l = -1;
  basement.i = 4; // (1,1)
  basement.name = "basement_slime";
  mapTemplate.characters.pushBack(std::move(basement));

  auto upstairs = model::MapCharacterPlacement{};
  upstairs.l = 0;
  upstairs.i = 4; // same tile index, different floor
  upstairs.name = "upstairs_slime";
  mapTemplate.characters.pushBack(std::move(upstairs));

  auto elsewhere = model::MapCharacterPlacement{};
  elsewhere.l = -1;
  elsewhere.i = 0; // (0,0)
  elsewhere.name = "corner_slime";
  mapTemplate.characters.pushBack(std::move(elsewhere));

  return mapTemplate;
}

void addMinimalDatabase(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);

  auto enemy = model::CharacterTemplate{};
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.name = "basement_slime";
  enemy.combat.hp = 10;
  database.addCharacterTemplate(enemy);

  enemy.name = "upstairs_slime";
  database.addCharacterTemplate(enemy);

  enemy.name = "corner_slime";
  database.addCharacterTemplate(enemy);

  auto party = model::CharacterTemplate{};
  party.type = model::CharacterTemplateType::TOWNSPERSON;
  party.name = "hero";
  database.addCharacterTemplate(party);

  model::MapGridTemplate grid;
  grid.name = "layer_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 3;
  grid.mapHeight = 3;
  grid.cells = {{"layer_char_map"}};
  database.addMapGridTemplate(grid);
}

model::CharacterInstance*
findByTemplate(model::ActiveMap& activeMap, const char* templateName) {
  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    if (activeMap.characters[i].templateName == templateName) {
      return &activeMap.characters[i];
    }
  }
  return nullptr;
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestCharacterMapLayer" << LOG_ENDL;
  auto ok = true;

  // createMapInstanceFromTemplate copies placement.l onto CharacterInstance.mapLayer
  {
    auto instance = model::createMapInstanceFromTemplate(makeLayeredCharacterMap());
    ok = assertEqual(static_cast<int>(instance.persistentState.characters.size()),
                     3,
                     "template characters.size") &&
         ok;

    auto foundBasement = false;
    auto foundUpstairs = false;
    auto foundCorner = false;
    for (const auto& character : instance.persistentState.characters) {
      if (character.templateName == "basement_slime") {
        foundBasement = true;
        ok = assertEqual(character.mapLayer, -1, "basement_slime.mapLayer") && ok;
        ok = assertEqual(character.x, 1, "basement_slime.x") && ok;
        ok = assertEqual(character.y, 1, "basement_slime.y") && ok;
      } else if (character.templateName == "upstairs_slime") {
        foundUpstairs = true;
        ok = assertEqual(character.mapLayer, 0, "upstairs_slime.mapLayer") && ok;
        ok = assertEqual(character.x, 1, "upstairs_slime.x") && ok;
        ok = assertEqual(character.y, 1, "upstairs_slime.y") && ok;
      } else if (character.templateName == "corner_slime") {
        foundCorner = true;
        ok = assertEqual(character.mapLayer, -1, "corner_slime.mapLayer") && ok;
      }
    }
    ok = assertTrue(foundBasement, "found basement_slime") && ok;
    ok = assertTrue(foundUpstairs, "found upstairs_slime") && ok;
    ok = assertTrue(foundCorner, "found corner_slime") && ok;
  }

  db::Database database;
  addMinimalDatabase(database);
  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  state.world.activeMap.gridId = "layer_grid";
  state.world.activeMap.mapLayer = 0;

  auto instance = model::createMapInstanceFromTemplate(makeLayeredCharacterMap());
  for (auto character : instance.persistentState.characters) {
    state.world.activeMap.characters.pushBack(std::move(character));
  }
  state.mapInstances[instance.templateName] = std::move(instance);

  // Active floor 0: same (x,y) basement NPC is invisible to lookups
  {
    auto* found = game::findCharacterAt(state.world.activeMap, 1, 1);
    ok = assertTrue(found != nullptr, "findCharacterAt layer 0 finds someone") && ok;
    ok = assertTrue(found != nullptr && found->templateName == "upstairs_slime",
                    "findCharacterAt layer 0 finds upstairs only") &&
         ok;

    auto all = game::findAllCharactersAt(state.world.activeMap, 1, 1);
    ok = assertEqual(static_cast<int>(all.size()), 1, "findAllCharactersAt layer 0 size") &&
         ok;
    if (!all.empty()) {
      ok = assertTrue(all[0]->templateName == "upstairs_slime",
                      "findAllCharactersAt layer 0 upstairs") &&
           ok;
    }

    ok = assertTrue(game::findCharacterAt(state.world.activeMap, 0, 0) == nullptr,
                    "findCharacterAt layer 0 ignores basement corner") &&
         ok;
  }

  // Switch to basement floor: opposite filter
  {
    state.world.activeMap.mapLayer = -1;
    auto* found = game::findCharacterAt(state.world.activeMap, 1, 1);
    ok = assertTrue(found != nullptr && found->templateName == "basement_slime",
                    "findCharacterAt layer -1 finds basement") &&
         ok;

    auto all = game::findAllCharactersAt(state.world.activeMap, 1, 1);
    ok = assertEqual(static_cast<int>(all.size()), 1, "findAllCharactersAt layer -1 size") &&
         ok;

    auto* corner = game::findCharacterAt(state.world.activeMap, 0, 0);
    ok = assertTrue(corner != nullptr && corner->templateName == "corner_slime",
                    "findCharacterAt layer -1 finds corner") &&
         ok;
  }

  // Pathfinding occupancy uses active layer only
  {
    state.world.activeMap.mapLayer = 0;
    auto* upstairs = findByTemplate(state.world.activeMap, "upstairs_slime");
    ok = assertTrue(upstairs != nullptr, "upstairs present for path test") && ok;

    // Tile (1,1) occupied upstairs — not pathable for a traveler on layer 0
    const auto blocked = game::collectReachableTiles(state.world.activeMap,
                                                     state.mapInstances,
                                                     0,
                                                     1,
                                                     1,
                                                     bmin::String{"traveler"},
                                                     database);
    auto canStepOntoUpstairs = false;
    for (size_t i = 0; i < blocked.size(); i++) {
      if (blocked[i].x == 1 && blocked[i].y == 1 && blocked[i].dist == 1) {
        canStepOntoUpstairs = true;
        break;
      }
    }
    ok = assertTrue(!canStepOntoUpstairs, "pathfinding blocked by same-layer NPC") && ok;

    // Basement occupant at (0,0) must not block layer 0
    const auto openCorner = game::collectReachableTiles(state.world.activeMap,
                                                        state.mapInstances,
                                                        1,
                                                        0,
                                                        1,
                                                        bmin::String{"traveler"},
                                                        database);
    auto canStepOntoCorner = false;
    for (size_t i = 0; i < openCorner.size(); i++) {
      if (openCorner[i].x == 0 && openCorner[i].y == 0 && openCorner[i].dist == 1) {
        canStepOntoCorner = true;
        break;
      }
    }
    ok = assertTrue(canStepOntoCorner, "pathfinding ignores other-layer NPC") && ok;
  }

  // placePartyAvatarAt syncs avatar.mapLayer to activeMap.mapLayer
  {
    state.world.activeMap.mapLayer = 0;
    auto member = model::CharacterPlayer{};
    member.instanceId = "party-avatar";
    member.name = "Hero";
    member.templateName = "hero";
    state.player.party.pushBack(std::move(member));

    auto* avatar =
        game::placePartyAvatarAt(state.world.activeMap, state.player, 2, 2, &database);
    ok = assertTrue(avatar != nullptr, "avatar placed") && ok;
    ok = assertEqual(avatar->mapLayer, 0, "avatar.mapLayer on layer 0") && ok;

    state.world.activeMap.mapLayer = -1;
    avatar = game::placePartyAvatarAt(state.world.activeMap, state.player, 2, 2, &database);
    ok = assertTrue(avatar != nullptr, "avatar moved") && ok;
    ok = assertEqual(avatar->mapLayer, -1, "avatar.mapLayer syncs on floor change") && ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestCharacterMapLayer assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestCharacterMapLayer completed successfully" << LOG_ENDL;
  return 0;
}
