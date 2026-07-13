#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/actions/world/WorldTalkDirection.hpp"
#include "bmin/String.h"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertFalse(bool cond, const char* label) {
  if (cond) {
    LOG(ERROR) << label << " expected false" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const bmin::String& expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected << "' but got '" << actual << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

model::MapInstance makeMap(int width, int height) {
  auto map = model::MapInstance{};
  map.width = width;
  map.height = height;
  map.tileLayerNumber = 0;
  return map;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldTalkDirection" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);

  auto talker = model::CharacterTemplate{};
  talker.name = "TalkNpc";
  talker.label = "Chatty One";
  talker.talk.talkName = "npc_talk_event";
  database.addCharacterTemplate(talker);

  auto silent = model::CharacterTemplate{};
  silent.name = "SilentNpc";
  silent.label = "Quiet One";
  database.addCharacterTemplate(silent);

  bool ok = true;

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.actionMode = model::WorldActionMode::TALK;

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.currentMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });
    state.world.currentMap.characters.pushBack(model::CharacterInstance{
        .id = "npc1",
        .templateName = "TalkNpc",
        .x = 3,
        .y = 2,
    });

    state::actions::WorldTalkDirection talkEast(1, 0);
    talkEast.execute(&state);

    ok = assertTrue(state.world.pendingSpecialEventId.has_value(),
                    "talk queues special event") &&
         ok;
    if (state.world.pendingSpecialEventId) {
      ok = assertEqualStr(*state.world.pendingSpecialEventId, "npc_talk_event",
                          "talk event id") &&
           ok;
    }
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "talk mode cleared") &&
         ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.actionMode = model::WorldActionMode::TALK;

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.currentMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });
    state.world.currentMap.characters.pushBack(model::CharacterInstance{
        .id = "npc2",
        .templateName = "SilentNpc",
        .x = 3,
        .y = 2,
    });

    state::actions::WorldTalkDirection talkEast(1, 0);
    talkEast.execute(&state);

    ok = assertFalse(state.world.pendingSpecialEventId.has_value(),
                     "silent npc does not queue event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "talk mode cleared for silent") &&
         ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.actionMode = model::WorldActionMode::TALK;

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.currentMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });

    state::actions::WorldTalkDirection talkEast(1, 0);
    talkEast.execute(&state);

    ok = assertFalse(state.world.pendingSpecialEventId.has_value(),
                     "empty tile does not queue event") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestWorldTalkDirection failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldTalkDirection completed successfully" << LOG_ENDL;
  return 0;
}
