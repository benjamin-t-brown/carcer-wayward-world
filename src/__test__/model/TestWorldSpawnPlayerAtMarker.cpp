#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/actions/world/WorldLoadMap.hpp"
#include "state/actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "model/TileTriggers.h"
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
                    const bmin::String& expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
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

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldSpawnPlayerAtMarker" << LOG_ENDL;

  bool ok = true;

  // MarkerPlayer on alinea_outsideAlinea1: i=337, width=30 → (7, 11)
  {
    auto tile = model::tileIndexToXY(337, 30);
    ok = assertEqual(tile.x, 7, "tileIndexToXY(337,30).x") && ok;
    ok = assertEqual(tile.y, 11, "tileIndexToXY(337,30).y") && ok;
  }

  {
    auto mapTemplate = model::CarcerMapTemplate{};
    mapTemplate.name = "fixture";
    mapTemplate.width = 30;
    mapTemplate.height = 30;
    {
      auto marker = model::MapMarkerPlacement{};
      marker.l = 0;
      marker.i = 337;
      marker.name = "MarkerPlayer";
      mapTemplate.markers.pushBack(std::move(marker));
    }
    {
      auto marker = model::MapMarkerPlacement{};
      marker.l = 0;
      marker.i = 10;
      marker.name = "Other";
      mapTemplate.markers.pushBack(std::move(marker));
    }

    const auto* marker =
        model::findMarkerOnTemplate(mapTemplate, bmin::String("MarkerPlayer"));
    ok = assertTrue(marker != nullptr, "findMarkerOnTemplate MarkerPlayer") && ok;
    if (marker) {
      ok = assertEqual(marker->i, 337, "MarkerPlayer.i") && ok;
      auto tile = model::tileIndexToXY(marker->i, mapTemplate.width);
      ok = assertEqual(tile.x, 7, "MarkerPlayer tile.x") && ok;
      ok = assertEqual(tile.y, 11, "MarkerPlayer tile.y") && ok;
    }
    ok = assertTrue(model::findMarkerOnTemplate(mapTemplate, bmin::String("missing")) ==
                        nullptr,
                    "findMarkerOnTemplate missing") &&
         ok;
  }

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    database.load();

    state::State state;
    state.player.party.pushBack(
        model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1")));
    state.player.currentPartyMemberIndex = 0;
    const auto partyId = state.player.party[0].instanceId;

    {
      auto loadMap = state::actions::WorldLoadMap("alinea_outsideAlinea1");
      loadMap.execute(&state);
    }
    {
      auto spawn = state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer");
      spawn.execute(&state);
    }

    const auto& characters = state.world.currentMap.characters;
    // alinea_outsideAlinea1 has NPC alinea_Claire; spawn adds party avatar.
    ok = assertEqual(static_cast<int>(characters.size()), 2, "characters.size after spawn") &&
         ok;
    const model::CharacterInstance* avatar = nullptr;
    const model::CharacterInstance* claire = nullptr;
    for (size_t i = 0; i < characters.size(); i++) {
      if (characters[i].id == partyId) {
        avatar = &characters[i];
      }
      if (characters[i].templateName == "alinea_Claire") {
        claire = &characters[i];
      }
    }
    ok = assertTrue(avatar != nullptr, "spawned avatar found") && ok;
    ok = assertTrue(claire != nullptr, "claire NPC found") && ok;
    if (avatar) {
      ok = assertEqual(avatar->x, 7, "spawned character.x") && ok;
      ok = assertEqual(avatar->y, 11, "spawned character.y") && ok;
    }
    ok = assertEqual(state.world.currentMap.tileLayerNumber, 0,
                     "tileLayerNumber after MarkerPlayer") &&
         ok;
    if (claire) {
      ok = assertEqual(claire->x, 4, "claire.x") && ok;
      ok = assertEqual(claire->y, 16, "claire.y") && ok;
    }

    // Idempotent re-spawn: still one avatar + NPCs
    {
      auto spawn = state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer");
      spawn.execute(&state);
    }
    ok = assertEqual(static_cast<int>(state.world.currentMap.characters.size()),
                     2,
                     "characters.size after re-spawn") &&
         ok;

    // Stairs1 is on layer 1 — spawn must update tileLayerNumber
    {
      auto spawn = state::actions::WorldSpawnPlayerAtMarker("Stairs1");
      spawn.execute(&state);
    }
    ok = assertEqual(state.world.currentMap.tileLayerNumber, 1,
                     "tileLayerNumber after Stairs1") &&
         ok;
    {
      const auto* stairsAvatar =
          model::findPartyAvatarOnMap(state.world.currentMap, state.player);
      ok = assertTrue(stairsAvatar != nullptr, "avatar after Stairs1") && ok;
      if (stairsAvatar) {
        // Stairs1 i=37, width=30 → (7, 1)
        ok = assertEqual(stairsAvatar->x, 7, "Stairs1 avatar.x") && ok;
        ok = assertEqual(stairsAvatar->y, 1, "Stairs1 avatar.y") && ok;
      }
    }
    ok = assertEqual(static_cast<int>(state.world.currentMap.characters.size()),
                     2,
                     "characters.size after Stairs1 spawn") &&
         ok;

    if (!ok) {
      LOG(ERROR) << "TestWorldSpawnPlayerAtMarker assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestWorldSpawnPlayerAtMarker completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
