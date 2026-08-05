#pragma once

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
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

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(x, y);
    const auto local = orch.activeMapCoordToInstanceCoord(x, y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      LOG(INFO) << TRANSLATE("You can't see there.") << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const model::CharacterInstance* target = nullptr;
    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& character = world.activeMap.characters[i];
      if (character.x != x || character.y != y) {
        continue;
      }
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

    if (!target) {
      LOG(INFO) << TRANSLATE("Talk: there is no one there.") << LOG_ENDL;
      return;
    }

    try {
      const auto& characterTemplate =
          database->getCharacterTemplate(bmin::toStringView(target->templateName));
      const auto& talkName = characterTemplate.talk.talkName;
      if (talkName.empty()) {
        LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
        return;
      }
      if (!database->getGameEvents().contains(talkName)) {
        LOG(ERROR) << "WorldTalkAt: talk event not found: " << talkName << LOG_ENDL;
        LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
        return;
      }
      state->triggers.pendingSpecialEventId = talkName;
    } catch (...) {
      LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
    }
  }

public:
  WorldTalkAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
