#pragma once

#include "db/Database.h"
#include "model/TileTriggers.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/StringInterop.h"

namespace state {

namespace actions {

// After Talk mode + a direction: start the adjacent character's talk special event
// (CharacterTemplate.talk.talkName) if present.
class WorldTalkDirection : public AbstractAction {
  int dx = 0;
  int dy = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldTalkDirection::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldTalkDirection::act: state is nullptr" << LOG_ENDL;
      return;
    }

    state->world.actionMode = model::WorldActionMode::NONE;

    auto& map = state->world.currentMap;
    const auto* avatar = model::findPartyAvatarOnMap(map, state->player);
    if (!avatar) {
      LOG(ERROR) << "WorldTalkDirection::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    const auto targetX = avatar->x + dx;
    const auto targetY = avatar->y + dy;
    if (targetX < 0 || targetY < 0 || targetX >= map.width || targetY >= map.height) {
      return;
    }

    const model::CharacterInstance* target = nullptr;
    for (size_t i = 0; i < map.characters.size(); i++) {
      const auto& character = map.characters[i];
      if (character.x == targetX && character.y == targetY) {
        // Prefer a character that has a talk event; otherwise remember the first occupant.
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
      LOG(INFO) << "Talk: there is no one there." << LOG_ENDL;
      return;
    }

    try {
      const auto& characterTemplate =
          database->getCharacterTemplate(bmin::toStringView(target->templateName));
      if (characterTemplate.talk.talkName.empty()) {
        LOG(INFO) << "Talk: they have nothing to say." << LOG_ENDL;
        return;
      }
      state->world.pendingSpecialEventId = characterTemplate.talk.talkName;
    } catch (...) {
      LOG(INFO) << "Talk: they have nothing to say." << LOG_ENDL;
    }
  }

public:
  WorldTalkDirection(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
