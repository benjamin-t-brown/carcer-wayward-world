#pragma once

#include "bmin/String.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.hpp"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

struct WorldSetActionModeCtx {
  bmin::String spellId;
  bmin::String chId;
};

class WorldSetActionMode : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSetActionMode; }
  model::WorldActionMode mode = model::WorldActionMode::NONE;
  WorldSetActionModeCtx ctx;

  void act() override {
    if (!state) {
      return;
    }
    state->world.actionMode = mode;
    if (mode == model::WorldActionMode::SPELL) {
      if (!ctx.spellId.empty()) {
        state->world.pendingSpellId = ctx.spellId;
      }
      if (!ctx.chId.empty()) {
        state->world.pendingChId = ctx.chId;
      }
    } else {
      state->world.pendingSpellId = bmin::String{};
    }
    if (mode == model::WorldActionMode::NONE) {
      state->world.actionAimTile.reset();
      return;
    }

    // Combat SPELL: aim under the active combat character (caster).
    if (mode == model::WorldActionMode::SPELL && state->world.combat.active &&
        !state->world.combat.activeCharacterId.empty()) {
      for (const auto& character : state->world.activeMap.characters) {
        if (character.id == state->world.combat.activeCharacterId) {
          state->world.actionAimTile = model::TileXY{character.x, character.y};
          return;
        }
      }
    }

    // EXAMINE / TALK / SPELL fallback: start aim under the party leader avatar.
    const auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (avatar) {
      state->world.actionAimTile = model::TileXY{avatar->x, avatar->y};
    } else {
      state->world.actionAimTile.reset();
    }
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode,
                              const WorldSetActionModeCtx& _ctx = {})
      : mode(_mode), ctx(_ctx) {}
};

} // namespace actions

} // namespace state
