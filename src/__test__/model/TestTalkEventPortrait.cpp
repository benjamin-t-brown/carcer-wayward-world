#include "db/Database.h"
#include "game/TalkEventPortrait.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/SpecialEvents.hpp"
#include "sdl2w/Logger.h"
#include "bmin/Map.h"
#include "bmin/String.h"

namespace {

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

model::GameEvent makeEvent(const char* id,
                           model::GameEventType type,
                           const char* icon) {
  auto event = model::GameEvent{};
  event.id = id;
  event.eventType = type;
  event.icon = icon;
  return event;
}

model::CharacterTemplate makeTalker(const char* name,
                                    const char* talkName,
                                    const char* portraitName) {
  auto character = model::CharacterTemplate{};
  character.name = name;
  character.talk.talkName = talkName;
  character.talk.portraitName = portraitName;
  return character;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestTalkEventPortrait" << LOG_ENDL;

  bool ok = true;
  bmin::Map<bmin::String, model::CharacterTemplate> characters;

  const auto talkEvent = makeEvent("npc_talk", model::GameEventType::TALK, "event_icon");
  const auto modalEvent =
      makeEvent("modal_event", model::GameEventType::MODAL, "modal_icon");

  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, characters),
                      "event_icon",
                      "talk without character uses event icon") &&
       ok;
  ok = assertEqualStr(game::resolveTalkEventPortrait(modalEvent, characters),
                      "modal_icon",
                      "modal uses event icon") &&
       ok;

  characters["empty_portrait"] = makeTalker("empty_portrait", "npc_talk", "");
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, characters),
                      "event_icon",
                      "empty character portrait falls back to event icon") &&
       ok;

  characters["claire"] = makeTalker("claire", "npc_talk", "portrait_claire");
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, characters),
                      "portrait_claire",
                      "character portrait wins for talk event") &&
       ok;
  ok = assertEqualStr(game::resolveTalkEventPortrait(modalEvent, characters),
                      "modal_icon",
                      "modal still uses event icon when characters exist") &&
       ok;

  db::Database database;
  database.addCharacterTemplate(makeTalker("claire", "npc_talk", "portrait_claire"));
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, &database),
                      "portrait_claire",
                      "database overload uses character portrait") &&
       ok;
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, nullptr),
                      "event_icon",
                      "null database falls back to event icon") &&
       ok;

  characters["barto"] = makeTalker("barto", "other_talk", "portrait_barto");
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, characters, "barto"),
                      "portrait_barto",
                      "aux character portrait replaces the talk owner") &&
       ok;
  ok = assertEqualStr(
      game::resolveTalkEventPortrait(talkEvent, characters, "portraits0_9"),
      "portraits0_9",
      "unknown aux name is used as a portrait sprite") &&
       ok;
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, characters, ""),
                      "portrait_claire",
                      "empty aux keeps the talk owner portrait") &&
       ok;

  characters["no_art"] = makeTalker("no_art", "unused_talk", "");
  ok = assertEqualStr(game::resolveTalkEventPortrait(talkEvent, characters, "no_art"),
                      "portrait_claire",
                      "aux character without portrait keeps the talk owner") &&
       ok;

  if (!ok) {
    LOG(ERROR) << "TestTalkEventPortrait failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestTalkEventPortrait completed successfully" << LOG_ENDL;
  return 0;
}
