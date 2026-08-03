#include "game/map/MapPersistence.h"
#include "bmin/StringInterop.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"

namespace game {
namespace {

// int tileCellIndex(const model::MapInstance& map, int x, int y) {
//   if (map.width <= 0 || x < 0 || y < 0 || x >= map.width || y >= map.height) {
//     return -1;
//   }
//   return y * map.width + x;
// }

// model::TileInstance* findTileAtCell(model::MapInstance& map, int layer, int x, int y) {
//   const auto index = tileCellIndex(map, x, y);
//   if (index < 0 || layer < 0 || static_cast<size_t>(layer) >= map.tiles.size()) {
//     return nullptr;
//   }
//   auto& layerTiles = map.tiles[static_cast<size_t>(layer)];
//   if (index >= static_cast<int>(layerTiles.size())) {
//     return nullptr;
//   }
//   return &layerTiles[static_cast<size_t>(index)];
// }

// bmin::DynArray<model::PersistentTileFieldRecord> captureTileFields(
//     const model::MapInstance& map) {
//   auto records = bmin::DynArray<model::PersistentTileFieldRecord>{};
//   for (size_t layerIdx = 0; layerIdx < map.tiles.size(); layerIdx++) {
//     const auto& layer = map.tiles[layerIdx];
//     for (size_t ti = 0; ti < layer.size(); ti++) {
//       const auto& tile = layer[ti];
//       if (tile.fields.empty()) {
//         continue;
//       }
//       auto record = model::PersistentTileFieldRecord{};
//       record.layer = static_cast<int>(layerIdx);
//       record.x = tile.x;
//       record.y = tile.y;
//       record.fields = tile.fields;
//       records.pushBack(std::move(record));
//     }
//   }
//   return records;
// }

// void applyTileFields(model::MapInstance& map,
//                      const bmin::DynArray<model::PersistentTileFieldRecord>& records) {
//   for (const auto& record : records) {
//     auto* tile = findTileAtCell(map, record.layer, record.x, record.y);
//     if (tile == nullptr) {
//       continue;
//     }
//     tile->fields = record.fields;
//   }
// }

// void applyDefeatedCharacters(
//     model::MapInstance& map,
//     const bmin::DynArray<model::DefeatedCharacterRecord>& defeated) {
//   if (defeated.empty()) {
//     return;
//   }

//   for (size_t i = 0; i < map.characters.size();) {
//     const auto& character = map.characters[i];
//     auto remove = false;
//     for (const auto& record : defeated) {
//       if (character.templateName != record.templateName) {
//         continue;
//       }
//       const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
//       const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;
//       if (spawnX == record.x && spawnY == record.y) {
//         remove = true;
//         break;
//       }
//     }
//     if (remove) {
//       map.characters.erase(i);
//     } else {
//       ++i;
//     }
//   }
// }

// void saveMapInstance(model::MapInstance& map, const db::Database& database) {
//   map.persistentState.explored = captureExploredMask(map);
//   map.persistentState.openedDoors = captureOpenedDoors(map, database);
//   map.persistentState.tileFields = captureTileFields(map);
// }

// void loadMapInstance(model::MapInstance& map) {
//   // Structural mutations first, then explored (visibility is recomputed on spawn/move).
//   applyOpenedDoors(map, map.persistentState.openedDoors);
//   applyTileFields(map, map.persistentState.tileFields);
//   applyDefeatedCharacters(map, map.persistentState.defeatedCharacters);
//   applyExploredMask(map, map.persistentState.explored);
// }

} // namespace

void createMapInstances(model::World& world, const db::Database& database) {
  world.mapInstances = bmin::Map<bmin::String, model::MapInstance>{};

  // getMapTemplates() returns const Map&; bmin::Map iteration needs a non-const begin().
  auto& templates = const_cast<bmin::Map<bmin::String, model::CarcerMapTemplate>&>(
      database.getMapTemplates());
  for (auto it = templates.begin(); it != templates.end(); ++it) {
    model::MapInstance instance = model::createMapInstanceFromTemplate(it->value);
    world.mapInstances[instance.templateName] = std::move(instance);
  }
}

} // namespace game
