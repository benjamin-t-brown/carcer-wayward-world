#include "game/TalkEventPortrait.h"
#include "db/Database.h"

namespace game {

bmin::String resolveTalkEventPortrait(
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates) {
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
                                      const db::Database* database) {
  if (!database) {
    return gameEvent.icon;
  }
  return resolveTalkEventPortrait(gameEvent, database->getCharacterTemplates());
}

} // namespace game
