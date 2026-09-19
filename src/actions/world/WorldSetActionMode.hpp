#pragma once

#include "bmin/String.h"
#include "game/map/Camera.h"
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
    auto& world = state->world;
    world.actionMode = mode;
    if (mode == model::WorldActionMode::SPELL) {
      if (!ctx.spellId.empty()) {
        world.pendingSpellId = ctx.spellId;
      }
      if (!ctx.chId.empty()) {
        world.pendingChId = ctx.chId;
      }
    } else {
      world.pendingSpellId = bmin::String{};
      // In combat, keep the camera on the last aim tile until the next turn
      // (SetActiveCombatCharacter restores Follow). Town has no turns, so snap back.
      if (world.camera.cameraMode == model::CameraMode::Aiming &&
          !world.combat.active) {
        world.camera.cameraMode = model::CameraMode::Follow;
      }
    }
    if (mode == model::WorldActionMode::NONE) {
      world.actionAimTile.reset();
      return;
    }

    bool placedCombatAim = false;
    // Combat SPELL: aim under the active combat character (caster).
    if (mode == model::WorldActionMode::SPELL && world.combat.active &&
        !world.combat.activeCharacterId.empty()) {
      for (const auto& character : world.activeMap.characters) {
        if (character.id == world.combat.activeCharacterId) {
          world.actionAimTile = model::TileXY{character.x, character.y};
          placedCombatAim = true;
          break;
        }
      }
    }

    if (!placedCombatAim) {
      // EXAMINE / TALK / SPELL fallback: start aim under the party leader avatar.
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
      if (avatar) {
        world.actionAimTile = model::TileXY{avatar->x, avatar->y};
      } else {
        world.actionAimTile.reset();
      }
    }

    if (mode == model::WorldActionMode::SPELL) {
      world.camera.cameraMode = model::CameraMode::Aiming;
      if (world.actionAimTile) {
        game::snapCameraToTile(world.camera, world.actionAimTile->x,
                               world.actionAimTile->y);
      }
    }
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode,
                              const WorldSetActionModeCtx& _ctx = {})
      : mode(_mode), ctx(_ctx) {}
};

} // namespace actions

} // namespace state
