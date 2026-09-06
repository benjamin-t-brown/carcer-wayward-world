module;
#include <utility>

export module carcer.actions.world:WorldSpawnProjectile;
export import carcer.state;

export {

namespace state::actions {

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

} // namespace state::actions

} // export
