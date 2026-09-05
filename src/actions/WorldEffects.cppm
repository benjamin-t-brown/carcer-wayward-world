module;
#include <cstddef>
#include <utility>

export module carcer.actions.world_effects;
export import carcer.state;
import carcer.game.map;
import carcer.model.templates;
import bmin.string_interop;

export {

namespace state::actions {

// Shared below the combat and world action modules so both domains can keep
// their deferred ActionBus effects without forming a module import cycle.
class WorldSpawnDamageParticle : public AbstractAction {
  bmin::String animationName;
  bmin::String text;
  int tileX = 0;
  int tileY = 0;
  int lifetimeMs = 0;

  void act() override {
    if (!state) {
      return;
    }

    model::DamageParticle particle;
    particle.animationName = animationName;
    particle.tileX = tileX;
    particle.tileY = tileY;
    particle.text = text;
    model::timerStructStart(particle.lifetime, lifetimeMs);
    state->world.activeMap.damageParticles.pushBack(std::move(particle));
  }

public:
  WorldSpawnDamageParticle(const bmin::String& _animationName,
                           const bmin::String& _text,
                           int _tileX,
                           int _tileY,
                           int _lifetimeMs)
      : animationName(_animationName),
        text(_text),
        tileX(_tileX),
        tileY(_tileY),
        lifetimeMs(_lifetimeMs) {}
};

class WorldSpawnProjectile : public AbstractAction {
  bmin::String animationName;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  int travelMs = 0;
  model::ProjectilePath projectilePath =
      model::ProjectilePath::PROJECTILE_PATH_NONE;

  void act() override {
    if (!state) {
      return;
    }

    model::WorldProjectile projectile;
    projectile.animationName = animationName;
    projectile.fromTileX = fromTileX;
    projectile.fromTileY = fromTileY;
    projectile.toTileX = toTileX;
    projectile.toTileY = toTileY;
    projectile.projectilePath = projectilePath;
    model::timerStructStart(projectile.travel, travelMs);
    state->world.activeMap.projectiles.pushBack(std::move(projectile));
  }

public:
  WorldSpawnProjectile(const bmin::String& _animationName,
                       float _fromTileX,
                       float _fromTileY,
                       float _toTileX,
                       float _toTileY,
                       int _travelMs,
                       model::ProjectilePath _projectilePath)
      : animationName(_animationName),
        fromTileX(_fromTileX),
        fromTileY(_fromTileY),
        toTileX(_toTileX),
        toTileY(_toTileY),
        travelMs(_travelMs),
        projectilePath(_projectilePath) {}
};

struct WorldSetActionModeCtx {
  bmin::String spellId;
  bmin::String chId;
};

class WorldSetActionMode : public AbstractAction {
  model::WorldActionMode mode = model::WorldActionMode::NONE;
  WorldSetActionModeCtx ctx;

  void act() override {
    if (!state) {
      return;
    }
    game::resolveWorldActionMode(
        state->world, state->player, mode, ctx.spellId, ctx.chId);
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode,
                              const WorldSetActionModeCtx& _ctx = {})
      : mode(_mode), ctx(_ctx) {}
};

} // namespace state::actions

} // export
