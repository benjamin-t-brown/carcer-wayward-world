#pragma once

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

// Confirm Talk at an absolute map tile: start that character's talk special event
// (CharacterTemplate.talk.talkName) if present.
class WorldTalkAt : public AbstractAction {
  int x = 0;
  int y = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldTalkAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldTalkAt::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& map = state->world.currentMap;
    if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
      return;
    }

    if (!game::isTileCurrentlyVisible(map, x, y)) {
      LOG(INFO) << TRANSLATE("You can't see there.") << LOG_ENDL;
      return;
    }

    state->world.actionMode = model::WorldActionMode::NONE;
    state->world.actionAimTile.reset();

    const model::CharacterInstance* target = nullptr;
    for (size_t i = 0; i < map.characters.size(); i++) {
      const auto& character = map.characters[i];
      if (character.x == x && character.y == y) {
        // Prefer a character that has a talk event; otherwise remember the first
        // occupant.
        if (!target) {
          target = &character;
        }
        try {
          const auto& characterTemplate =
              database->getCharacterTemplate(bmin::toStringView(character.templateName));
          if (!characterTemplate.talk.talkName.empty()) {
            target = &character;
            break;
          }
        } catch (...) {
        }
      }
    }

    if (!target) {
      LOG(INFO) << TRANSLATE("Talk: there is no one there.") << LOG_ENDL;
      return;
    }

    try {
      const auto& characterTemplate =
          database->getCharacterTemplate(bmin::toStringView(target->templateName));
      if (characterTemplate.talk.talkName.empty()) {
        LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
        return;
      }
      state->triggers.pendingSpecialEventId = characterTemplate.talk.talkName;
    } catch (...) {
      LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
    }
  }

public:
  WorldTalkAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
