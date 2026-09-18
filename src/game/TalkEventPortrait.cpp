#include "game/TalkEventPortrait.h"
#include "db/Database.h"

namespace game {

namespace {

bmin::String portraitForCharacterName(
    const bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates,
    const bmin::String& characterName) {
  const auto it = characterTemplates.find(characterName);
  if (it == characterTemplates.end()) {
    return {};
  }
  return it->value.talk.portraitName;
}

} // namespace

bmin::String resolveTalkEventPortrait(
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates,
    const bmin::String& auxName) {
  if (!auxName.empty()) {
    const auto auxPortrait = portraitForCharacterName(characterTemplates, auxName);
    if (!auxPortrait.empty()) {
      return auxPortrait;
    }
    if (characterTemplates.find(auxName) == characterTemplates.end()) {
      return auxName;
    }
  }

  if (gameEvent.eventType == model::GameEventType::TALK) {
    for (auto it = characterTemplates.begin(); it != characterTemplates.end(); ++it) {
      const auto& talk = (*it).value.talk;
      if (talk.talkName == gameEvent.id && !talk.portraitName.empty()) {
        return talk.portraitName;
      }
    }
  }
  return gameEvent.icon;
}

bmin::String resolveTalkEventPortrait(const model::GameEvent& gameEvent,
                                      const db::Database* database,
                                      const bmin::String& auxName) {
  if (!database) {
    if (!auxName.empty()) {
      return auxName;
    }
    return gameEvent.icon;
  }
  return resolveTalkEventPortrait(gameEvent, database->getCharacterTemplates(), auxName);
}

} // namespace game
