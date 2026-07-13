#include "db/Database.h"
#include "db/loaders/LoadMapTemplates.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const char* expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual.cStr()
               << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestCreateMapInstanceFromTemplate" << LOG_ENDL;

  try {
    bmin::Map<bmin::String, model::CarcerMapTemplate> maps;
    db::loadMapTemplates("__test__/assets/maps-flat-fixture.json", maps);

    const auto it = maps.find(bmin::String("flat_test_map"));
    if (it == maps.end()) {
      LOG(ERROR) << "Missing flat_test_map" << LOG_ENDL;
      return 1;
    }

    const model::CarcerMapTemplate& mapTemplate = it->value;
    auto instance = model::createMapInstanceFromTemplate(mapTemplate);

    bool ok = true;
    ok = assertEqual(instance.width, 2, "instance.width") && ok;
    ok = assertEqual(instance.height, 2, "instance.height") && ok;
    ok = assertEqual(instance.spriteWidth, 28, "instance.spriteWidth") && ok;
    ok = assertEqual(instance.spriteHeight, 32, "instance.spriteHeight") && ok;
    ok = assertEqual(static_cast<int>(instance.mapType),
                     static_cast<int>(model::MapType::TOWN),
                     "instance.mapType") &&
         ok;
    ok = assertEqual(static_cast<int>(instance.turnMode),
                     static_cast<int>(model::TurnMode::TURN_TOWN),
                     "instance.turnMode") &&
         ok;
    ok = assertEqualStr(instance.id, "flat_test_map", "instance.id") && ok;
    ok = assertEqualStr(instance.templateName, "flat_test_map", "instance.templateName") &&
         ok;
    ok = assertEqualStr(instance.label, "Flat Test Map", "instance.label") && ok;

    // Fixture: characters [{l:0,i:1,name:testNpc}] → tile (1,0)
    ok = assertEqual(static_cast<int>(instance.characters.size()),
                     1,
                     "instance.characters.size") &&
         ok;
    if (!instance.characters.empty()) {
      ok = assertEqualStr(instance.characters[0].name, "testNpc", "character.name") &&
           ok;
      ok = assertEqualStr(
               instance.characters[0].templateName, "testNpc", "character.templateName") &&
           ok;
      ok = assertEqual(instance.characters[0].x, 1, "character.x") && ok;
      ok = assertEqual(instance.characters[0].y, 0, "character.y") && ok;
      ok = assertTrue(!instance.characters[0].id.empty(), "character.id non-empty") && ok;
    }

    // Fixture: items [{l:0,i:2,name:BeerPappysLager,quantity:2}] → tile (0,1)
    ok = assertEqual(static_cast<int>(instance.items.size()), 1, "instance.items.size") &&
         ok;
    if (!instance.items.empty()) {
      ok = assertEqualStr(
               instance.items[0].itemTemplateName, "BeerPappysLager", "item.itemTemplateName") &&
           ok;
      ok = assertEqual(instance.items[0].quantity, 2, "item.quantity") && ok;
      ok = assertEqual(instance.items[0].x, 0, "item.x") && ok;
      ok = assertEqual(instance.items[0].y, 1, "item.y") && ok;
      ok = assertTrue(!instance.items[0].id.empty(), "item.id non-empty") && ok;
    }

    if (!mapHasLayer(instance.tiles, 0)) {
      LOG(ERROR) << "instance.tiles missing layer 0" << LOG_ENDL;
      return 1;
    }
    auto& layer0 = instance.tiles[0];
    ok = assertEqual(static_cast<int>(layer0.size()), 4, "instance.tiles[0].size") && ok;

    // Flat pairs: [1,5, 1,6, 1,7, 1,8] → cell 1 (x=1,y=0) is terrain0 / tileId 6
    ok = assertEqual(layer0[1].x, 1, "layer0[1].x") && ok;
    ok = assertEqual(layer0[1].y, 0, "layer0[1].y") && ok;
    ok = assertEqual(layer0[1].tileId, 6, "layer0[1].tileId") && ok;
    ok = assertEqualStr(layer0[1].tilesetName, "terrain0", "layer0[1].tilesetName") && ok;

    ok = assertEqual(layer0[0].tileId, 5, "layer0[0].tileId") && ok;
    ok = assertEqualStr(layer0[0].tilesetName, "terrain0", "layer0[0].tilesetName") && ok;
    ok = assertEqual(layer0[3].x, 1, "layer0[3].x") && ok;
    ok = assertEqual(layer0[3].y, 1, "layer0[3].y") && ok;
    ok = assertEqual(layer0[3].tileId, 8, "layer0[3].tileId") && ok;

    // Fixture event + travel on tile i:0 (last-wins both present)
    ok = assertTrue(layer0[0].eventTrigger.has_value(), "layer0[0].eventTrigger") && ok;
    if (layer0[0].eventTrigger) {
      ok = assertEqualStr(
               layer0[0].eventTrigger->eventId, "test_event", "eventTrigger.eventId") &&
           ok;
      ok = assertEqual(static_cast<int>(layer0[0].eventTrigger->isLookTrigger),
                       1,
                       "eventTrigger.isLookTrigger") &&
           ok;
    }
    ok = assertTrue(layer0[0].travelTrigger.has_value(), "layer0[0].travelTrigger") && ok;
    if (layer0[0].travelTrigger) {
      ok = assertEqualStr(layer0[0].travelTrigger->destinationMapName,
                          "other_map",
                          "travelTrigger.destinationMapName") &&
           ok;
      ok = assertEqualStr(layer0[0].travelTrigger->destinationMarkerName,
                          "door",
                          "travelTrigger.destinationMarkerName") &&
           ok;
      ok = assertEqual(layer0[0].travelTrigger->destinationX, 1, "travelTrigger.destinationX") &&
           ok;
      ok = assertEqual(layer0[0].travelTrigger->destinationY, 2, "travelTrigger.destinationY") &&
           ok;
    }

    if (!ok) {
      LOG(ERROR) << "createMapInstanceFromTemplate assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestCreateMapInstanceFromTemplate completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
