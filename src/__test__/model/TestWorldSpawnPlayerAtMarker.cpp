#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/world/WorldLoadActiveMap.hpp"
#include "state/actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "game/map/TileTriggers.h"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
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

bool assertEqualStr(const bmin::String& actual, const char* expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual.cStr()
               << LOG_ENDL;
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

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    database.load();

    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    state.player.party.pushBack(
        model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1")));
    state.player.currentPartyMemberIndex = 0;
    const auto partyId = state.player.party[0].instanceId;

    {
      auto loadMap = state::actions::WorldLoadActiveMap("alinea_outsideAlinea1");
      loadMap.execute(&state);
    }
    ok = assertEqualStr(state.world.activeMap.gridId, "OutsideAlinea",
                        "activeMap.gridId after load") &&
         ok;

    {
      auto spawn = state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer");
      spawn.execute(&state);
    }

    // alinea_outsideAlinea1 is at grid (1,0) → world offset (30,0)
    const model::CharacterInstance* avatar = nullptr;
    const model::CharacterInstance* claire = nullptr;
    const model::CharacterInstance* goblin = nullptr;
    for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
      const auto& ch = state.world.activeMap.characters[i];
      if (ch.id == partyId) {
        avatar = &ch;
      }
      if (ch.templateName == "alinea_Claire") {
        claire = &ch;
      }
      if (ch.templateName == "goblinTest") {
        goblin = &ch;
      }
    }
    ok = assertTrue(avatar != nullptr, "spawned avatar found") && ok;
    ok = assertTrue(claire != nullptr, "claire NPC found") && ok;
    ok = assertTrue(goblin != nullptr, "goblinTest NPC found") && ok;
    if (avatar) {
      ok = assertEqual(avatar->x, 37, "spawned character.x world") && ok;
      ok = assertEqual(avatar->y, 11, "spawned character.y world") && ok;
    }
    ok = assertEqual(state.world.activeMap.mapLayer, 0,
                     "mapLayer after MarkerPlayer") &&
         ok;
    if (claire) {
      ok = assertEqual(claire->x, 34, "claire.x world") && ok;
      ok = assertEqual(claire->y, 16, "claire.y world") && ok;
    }
    if (goblin) {
      ok = assertEqual(goblin->x, 48, "goblinTest.x world") && ok;
      ok = assertEqual(goblin->y, 11, "goblinTest.y world") && ok;
    }

    const auto countAfterSpawn =
        static_cast<int>(state.world.activeMap.characters.size());
    {
      auto spawn = state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer");
      spawn.execute(&state);
    }
    ok = assertEqual(static_cast<int>(state.world.activeMap.characters.size()),
                     countAfterSpawn,
                     "characters.size after re-spawn") &&
         ok;

    {
      auto spawn = state::actions::WorldSpawnPlayerAtMarker("Stairs1");
      spawn.execute(&state);
    }
    ok = assertEqual(state.world.activeMap.mapLayer, 1, "mapLayer after Stairs1") && ok;
    {
      const auto* stairsAvatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      ok = assertTrue(stairsAvatar != nullptr, "avatar after Stairs1") && ok;
      if (stairsAvatar) {
        ok = assertEqual(stairsAvatar->x, 37, "Stairs1 avatar.x world") && ok;
        ok = assertEqual(stairsAvatar->y, 1, "Stairs1 avatar.y world") && ok;
      }
    }

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
