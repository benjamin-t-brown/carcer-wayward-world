module;
#include <cmath>
#include <utility>
#include <cstddef>
#include <cstdlib>

export module carcer.actions:world;
export import carcer.state;
export import :combat;
import sdl2w;
import carcer.game.map;
import bmin.string_interop;
import :general;
import carcer.game.combat;
#include "macros.h"

export {

namespace state {

namespace actions {

class WorldExamineAt : public AbstractAction {
  sdl2w::Window* window = nullptr;
  int x = 0;
  int y = 0;

  static bool isAdjacentOrSame(int ax, int ay, int bx, int by) {
    return std::abs(ax - bx) <= 1 && std::abs(ay - by) <= 1;
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldExamineAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldExamineAt::act: state is nullptr" << LOG_ENDL;
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
      LOG(INFO) << "You can't see there." << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const auto* tile = game::tileAtCurrentLayer(*map, local.x, local.y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresLook) {
      state->triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    const bool isContainer =
        tile != nullptr && game::isTileEffectivelyContainer(*tile, *database);
    if (isContainer) {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
      const bool adjacent =
          avatar != nullptr && isAdjacentOrSame(avatar->x, avatar->y, x, y);
      if (adjacent && window) {
        const auto contents =
            game::collectItemsAtActiveMapTile(world.activeMap, x, y);
        if (contents.empty()) {
          LOG(INFO) << TRANSLATE("Nothing inside.") << LOG_ENDL;
          return;
        }
        pushLayerRequest(*state, LayerRequest{.id = LayerId::PickUp, .x = x, .y = y});
        return;
      }
      LOG(INFO) << game::formatExamineMessage(
                       *map, world.activeMap, x, y, local.x, local.y, *database)
                << LOG_ENDL;
      LOG(INFO) << TRANSLATE("You need to get closer to look inside.") << LOG_ENDL;
      return;
    }

    LOG(INFO) << game::formatExamineMessage(
                     *map, world.activeMap, x, y, local.x, local.y, *database)
              << LOG_ENDL;
  }

public:
  WorldExamineAt(int _x, int _y) : x(_x), y(_y) {}
  WorldExamineAt(sdl2w::Window* _window, int _x, int _y)
      : window(_window), x(_x), y(_y) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldInteractAt : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    const auto* avatar =
        game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
    if (!avatar || world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(avatar->x, avatar->y);
    const auto local = orch.activeMapCoordToInstanceCoord(avatar->x, avatar->y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    game::queueActionTravelAtStanding(state->triggers, *map, local.x, local.y);
  }
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldLoadActiveMap : public AbstractAction {
  bmin::String gridId;

  void saveCurrentMapToPersistentState() {
    auto& localState = *state;

    auto previousGridId = localState.world.activeMap.gridId;
    if (previousGridId.empty()) {
      return;
    }
    game::ActiveMapOrchestrator previousActiveMap;
    previousActiveMap.fetchMapGrid(previousGridId);

    for (auto ch : localState.world.activeMap.characters) {
      if (model::isPartyMember(localState.player, ch.id)) {
        continue;
      }
      auto* map = previousActiveMap.getMapInstanceAt(ch.x, ch.y);
      if (!map) {
        map = previousActiveMap.getDefaultMapInstance();
      }
      if (!map) {
        continue;
      }
      const auto local = previousActiveMap.activeMapCoordToInstanceCoord(ch.x, ch.y);
      if (local.valid) {
        ch.x = local.x;
        ch.y = local.y;
      }
      map->persistentState.characters.pushBack(std::move(ch));
    }
    for (auto item : localState.world.activeMap.items) {
      auto* map = previousActiveMap.getMapInstanceAt(item.x, item.y);
      if (!map) {
        map = previousActiveMap.getDefaultMapInstance();
      }
      if (!map) {
        continue;
      }
      const auto local = previousActiveMap.activeMapCoordToInstanceCoord(item.x, item.y);
      if (local.valid) {
        item.x = local.x;
        item.y = local.y;
      }
      map->persistentState.items.pushBack(std::move(item));
    }
  }

  void act() override {
    auto& localState = *state;

    auto* database = getDatabase();
    if (!database) {
      return;
    }

    const auto resolvedGridId = game::resolveGridIdForMapOrGrid(*database, gridId);
    if (resolvedGridId.empty()) {
      return;
    }

    if (localState.mapInstances.empty()) {
      game::createMapInstances(localState, *database);
    }

    saveCurrentMapToPersistentState();

    localState.world.activeMap = {};
    localState.world.activeMap.gridId = resolvedGridId;
    localState.world.camera.camX = 0;
    localState.world.camera.camY = 0;
    localState.world.camera.cameraMode = model::CameraMode::Follow;
    localState.world.camera.cameraFollowCharacterId = bmin::String{};
    localState.world.actionMode = model::WorldActionMode::NONE;
    localState.world.actionAimTile.reset();
    localState.world.pendingSpellId = bmin::String{};

    game::ActiveMapOrchestrator activeMap;
    activeMap.fetchMapGrid(resolvedGridId);
    auto& grid = activeMap.getMapGrid();
    for (int y = 0; y < grid.gridHeight; y++) {
      for (int x = 0; x < grid.gridWidth; x++) {
        const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (mapName.empty()) {
          continue;
        }
        auto it = localState.mapInstances.find(mapName);
        if (it == localState.mapInstances.end()) {
          continue;
        }
        auto& map = it->value;
        auto& persistentState = map.persistentState;

        // Drop defeated characters before hoisting.
        for (size_t ci = 0; ci < persistentState.characters.size();) {
          const auto& character = persistentState.characters[ci];
          auto remove = false;
          for (const auto& record : persistentState.defeatedCharacters) {
            if (character.templateName != record.templateName) {
              continue;
            }
            const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
            const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;
            if (spawnX == record.x && spawnY == record.y) {
              remove = true;
              break;
            }
          }
          if (remove) {
            persistentState.characters.erase(ci);
          } else {
            ++ci;
          }
        }

        auto* database = getDatabase();
        for (auto character : persistentState.characters) {
          const auto worldLoc =
              activeMap.instanceCoordToActiveMapCoord(mapName, character.x, character.y);
          if (worldLoc.valid) {
            character.x = worldLoc.x;
            character.y = worldLoc.y;
          }
          if (database) {
            model::tryApplyCharacterTemplateToInstance(character, *database);
          }
          localState.world.activeMap.characters.pushBack(std::move(character));
        }
        for (auto item : persistentState.items) {
          const auto worldLoc =
              activeMap.instanceCoordToActiveMapCoord(mapName, item.x, item.y);
          if (worldLoc.valid) {
            item.x = worldLoc.x;
            item.y = worldLoc.y;
          }
          localState.world.activeMap.items.pushBack(std::move(item));
        }
        persistentState.characters.clear();
        persistentState.items.clear();
      }
    }
  }

public:
  WorldLoadActiveMap(const bmin::String& gridId) : gridId(gridId) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldMoveActionAim : public AbstractAction {
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (state->world.actionMode == model::WorldActionMode::NONE) {
      return;
    }
    if (!state->world.actionAimTile) {
      return;
    }
    if (state->world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(state->world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto& aim = *state->world.actionAimTile;
    auto nextX = aim.x + dx;
    auto nextY = aim.y + dy;
    if (nextX < 0) {
      nextX = 0;
    } else if (nextX >= total.x) {
      nextX = total.x - 1;
    }
    if (nextY < 0) {
      nextY = 0;
    } else if (nextY >= total.y) {
      nextY = total.y - 1;
    }
    aim.x = nextX;
    aim.y = nextY;
  }

public:
  WorldMoveActionAim(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldSetActionAim : public AbstractAction {
  int x = 0;
  int y = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (state->world.actionMode == model::WorldActionMode::NONE) {
      return;
    }
    if (state->world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(state->world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto nextX = x;
    auto nextY = y;
    if (nextX < 0) {
      nextX = 0;
    } else if (nextX >= total.x) {
      nextX = total.x - 1;
    }
    if (nextY < 0) {
      nextY = 0;
    } else if (nextY >= total.y) {
      nextY = total.y - 1;
    }

    if (state->world.actionAimTile && state->world.actionAimTile->x == nextX &&
        state->world.actionAimTile->y == nextY) {
      return;
    }
    state->world.actionAimTile = model::TileXY{nextX, nextY};
  }

public:
  WorldSetActionAim(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

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

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldSetCamera : public AbstractAction {
  int camX = 0;
  int camY = 0;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camera.camX = camX;
    state->world.camera.camY = camY;
  }

public:
  WorldSetCamera(int _camX, int _camY) : camX(_camX), camY(_camY) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldSetCameraMode : public AbstractAction {
  model::CameraMode cameraMode = model::CameraMode::Follow;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camera.cameraMode = cameraMode;
  }

public:
  explicit WorldSetCameraMode(model::CameraMode _cameraMode)
      : cameraMode(_cameraMode) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

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
      : animationName((_animationName)),
        text((_text)),
        tileX(_tileX),
        tileY(_tileY),
        lifetimeMs(_lifetimeMs) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

// Spawns (or re-spawns) one player avatar CharacterInstance at a named marker on
// the active map grid. Does not move the camera.
class WorldSpawnPlayerAtMarker : public AbstractAction {
  bmin::String markerName;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: no active map loaded" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto& grid = orch.getMapGrid();

    game::ActiveMapMarker found{};
    for (int y = 0; y < grid.gridHeight && !found.valid; ++y) {
      for (int x = 0; x < grid.gridWidth && !found.valid; ++x) {
        const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (mapName.empty()) {
          continue;
        }
        found = orch.findMarker(mapName, markerName);
      }
    }

    if (!found.valid) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: marker not found: " << markerName
                 << LOG_ENDL;
      return;
    }

    world.activeMap.mapLayer = found.layer;

    if (!game::placePartyAvatarAt(
            world.activeMap, state->player, found.x, found.y, database)) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: party is empty" << LOG_ENDL;
      return;
    }

    game::updateActiveMapVisibilityFromPlayer(world, found.x, found.y, *database);
  }

public:
  explicit WorldSpawnPlayerAtMarker(bmin::String _markerName = "MarkerPlayer")
      : markerName(std::move(_markerName)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

// Places the current party avatar at world tile coordinates on the active map.
class WorldSpawnPlayerAtXY : public AbstractAction {
  int destX = 0;
  int destY = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: no active map loaded" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: destination out of bounds" << LOG_ENDL;
      return;
    }

    if (!game::placePartyAvatarAt(
            world.activeMap, state->player, destX, destY, database)) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: party is empty" << LOG_ENDL;
      return;
    }

    game::updateActiveMapVisibilityFromPlayer(world, destX, destY, *database);
  }

public:
  WorldSpawnPlayerAtXY(int _destX, int _destY) : destX(_destX), destY(_destY) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldSpawnProjectile : public AbstractAction {
  bmin::String animationName;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  int travelMs = 0;
  // affects the "height" of the projectile, the y offset of the projectile on a sin wave
  // from start to end
  // - NONE means lerp directly to target
  // - SHORT means offset by half tile height (scaled)
  // - MEDIUM means offset by full tile height (scaled)
  // - TALL means offset by 2x tile height (scaled)
  model::ProjectilePath projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;

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
    // projectile.anim = sdl2w::Animation::create(animationName);
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

} // namespace actions

} // namespace state

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

namespace state {

namespace actions {

// Applies town-mode party HP change (victim need not be on the active map).
class ModifyPartyMemberHp : public AbstractAction {
  bmin::String instanceId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    model::modifyPartyMemberHp(state->player, instanceId, delta);
  }

public:
  ModifyPartyMemberHp(bmin::String _instanceId, int _delta)
      : instanceId(std::move(_instanceId)), delta(_delta) {}
};

// Town melee with the same swing / particle / reset timing as combat melee.
// Damages a random living party member; FX play on the party avatar tile.
class PerformTownMeleeAttack : public CombatAction {
  bmin::String attackerId;

  static model::CharacterPlayer* pickRandomLivingPartyMember(model::Player& player) {
    bmin::DynArray<model::CharacterPlayer*> living;
    for (size_t i = 0; i < player.party.size(); i++) {
      if (player.party[i].currentHp > 0) {
        living.pushBack(&player.party[i]);
      }
    }
    if (living.empty()) {
      return nullptr;
    }
    const auto index = static_cast<size_t>(std::rand() % static_cast<int>(living.size()));
    return living[index];
  }

  void act() override {
    if (!state) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    auto* attacker = orch.findCharacterById(attackerId);
    auto* avatar = game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (attacker == nullptr || avatar == nullptr) {
      return;
    }

    auto* victim = pickRandomLivingPartyMember(state->player);
    if (victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, avatar->x, avatar->y);

    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertAction(new PlaySound("punch1"), 0);
      insertAction(nullptr, 75);
      insertAction(new ModifyPartyMemberHp(victim->instanceId, -model::COMBAT_MELEE_DAMAGE),
                         0);
      insertAction(new WorldSpawnDamageParticle("splash_attack",
                                                      bmin::toString(model::COMBAT_MELEE_DAMAGE),
                                                      avatar->x,
                                                      avatar->y,
                                                      500),
                         0);
      insertAction(nullptr, 500);
      LOG(INFO) << "TownMeleeAttack: " << attackerId << " hit " << victim->instanceId
                << " for " << model::COMBAT_MELEE_DAMAGE << LOG_ENDL;
    } else {
      insertAction(new PlaySound("whip"), 0);
      insertAction(nullptr, 300);
      LOG(DEBUG) << "TownMeleeAttack: miss by " << attackerId << " vs party member "
                 << victim->instanceId << LOG_ENDL;
    }
    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
  }

public:
  explicit PerformTownMeleeAttack(bmin::String _attackerId)
      : attackerId(std::move(_attackerId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldSpawnPlayer : public AbstractAction {
  bmin::String mapName;
  bmin::String markerName;
  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayer::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayer::act: state is nullptr" << LOG_ENDL;
      return;
    }

    const auto gridId = game::resolveGridIdForMapOrGrid(*database, mapName);
    if (gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayer::act: could not resolve grid for " << mapName
                 << LOG_ENDL;
      return;
    }

    WorldLoadActiveMap(gridId).execute(state);
    WorldSpawnPlayerAtMarker(markerName).execute(state);
  }

public:
  explicit WorldSpawnPlayer(const bmin::String& _mapName, const bmin::String& _markerName)
      : mapName(_mapName), markerName(_markerName) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class WorldTravel : public AbstractAction {
  model::TravelTrigger travel;

  void act() override {
    if (!state) {
      return;
    }
    if (travel.destinationMapName.empty()) {
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      return;
    }

    const auto gridId =
        game::resolveGridIdForMapOrGrid(*database, travel.destinationMapName);
    if (gridId.empty()) {
      LOG(ERROR) << "WorldTravel::act: could not resolve grid for "
                 << travel.destinationMapName << LOG_ENDL;
      return;
    }

    // Same grid: teleport only. Different grid: unload/reload active map entities.
    if (state->world.activeMap.gridId != gridId) {
      WorldLoadActiveMap(gridId).execute(state);
    }

    auto usedMarker = false;
    if (!travel.destinationMarkerName.empty()) {
      game::ActiveMapOrchestrator orch;
      orch.fetchMapGrid(gridId);
      const auto marker =
          orch.findMarker(travel.destinationMapName, travel.destinationMarkerName);
      if (marker.valid) {
        state->world.activeMap.mapLayer = marker.layer;
        WorldSpawnPlayerAtXY(marker.x, marker.y).execute(state);
        usedMarker = true;
      } else {
        LOG(WARN) << "WorldTravel::act: marker not found on destination map, "
                     "falling back to XY: "
                  << travel.destinationMarkerName << LOG_ENDL;
      }
    }

    if (!usedMarker) {
      state->world.activeMap.mapLayer = travel.destinationLayer;
      // destinationX/Y are local to destinationMapName — convert to world.
      game::ActiveMapOrchestrator orch;
      orch.fetchMapGrid(gridId);
      const auto worldLoc = orch.instanceCoordToActiveMapCoord(
          travel.destinationMapName, travel.destinationX, travel.destinationY);
      if (worldLoc.valid) {
        WorldSpawnPlayerAtXY(worldLoc.x, worldLoc.y).execute(state);
      } else {
        WorldSpawnPlayerAtXY(travel.destinationX, travel.destinationY).execute(state);
      }
    }
  }

public:
  explicit WorldTravel(model::TravelTrigger _travel) : travel(std::move(_travel)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class ClearTownEnemyAiResolving : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    state->world.resolvingTownEnemyAi = false;
  }
};

// One agitated enemy: optional seek step, then town melee if adjacent.
class TownEnemySeekAndMelee : public CombatAction {
  bmin::String enemyId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    auto* enemy = orch.findCharacterById(enemyId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (enemy == nullptr || avatar == nullptr) {
      return;
    }

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(new PerformTownMeleeAttack(enemyId), 0);
      return;
    }

    auto dx = 0;
    auto dy = 0;
    if (!game::chooseSeekStepToward(
            state->world.activeMap, *enemy, avatar->x, avatar->y, *database, dx, dy)) {
      return;
    }

    model::updateCharacterFacingFromMove(*enemy, dx, dy);
    enemy->x += dx;
    enemy->y += dy;
    LOG(DEBUG) << "TownEnemyAi: " << enemyId << " stepped (" << dx << ", " << dy << ")"
               << LOG_ENDL;

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(new PerformTownMeleeAttack(enemyId), 0);
    }
  }

public:
  explicit TownEnemySeekAndMelee(bmin::String _enemyId) : enemyId(std::move(_enemyId)) {}
};

// Spotting + one town action per agitated enemy (queued with combat-style delays).
class TownEnemyAiAfterPlayerMove : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto& world = state->world;
    if (world.combat.active) {
      world.resolvingTownEnemyAi = false;
      return;
    }

    world.resolvingTownEnemyAi = true;
    // Held-move stays active; LayerWorld pauses repeats while this flag is set.

    game::updateEnemySpotting(world, state->player);

    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& character = world.activeMap.characters[i];
      if (!character.agitated) {
        continue;
      }
      if (!model::characterInstanceIsEnemy(character)) {
        continue;
      }
      if (character.behaviorName != "IMMOBILE_UNTIL_ENEMY_SPOTTED") {
        continue;
      }
      if (character.combatBehaviorTown != model::CombatBehaviorName::SEEK_AND_MELEE) {
        continue;
      }
      insertAction(new TownEnemySeekAndMelee(character.id), 0);
    }

    insertAction(new ClearTownEnemyAiResolving(), 0);
  }
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

// Moves the current party avatar by (dx, dy) tiles, or opens a closed door on bump.
class WorldMovePlayer : public CombatAction {
  int dx = 0;
  int dy = 0;

  const char* moveDirectionLabel(int dx, int dy) {
    if (dx < 0 && dy < 0) {
      return "nw";
    }
    if (dx == 0 && dy < 0) {
      return "n";
    }
    if (dx > 0 && dy < 0) {
      return "ne";
    }
    if (dx < 0 && dy == 0) {
      return "w";
    }
    if (dx > 0 && dy == 0) {
      return "e";
    }
    if (dx < 0 && dy > 0) {
      return "sw";
    }
    if (dx == 0 && dy > 0) {
      return "s";
    }
    if (dx > 0 && dy > 0) {
      return "se";
    }
    return "?";
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldMovePlayer::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldMovePlayer::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }
    if (!world.combat.active && world.resolvingTownEnemyAi) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto& player = state->player;
    if (player.party.empty()) {
      LOG(ERROR) << "WorldMovePlayer::act: party is empty" << LOG_ENDL;
      return;
    }

    auto* avatar = game::findPartyAvatarOnActiveMap(world.activeMap, player);
    if (!avatar) {
      LOG(ERROR) << "WorldMovePlayer::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    model::updateCharacterFacingFromMove(*avatar, dx, dy);

    const auto destX = avatar->x + dx;
    const auto destY = avatar->y + dy;
    LOG(DEBUG) << "WorldMovePlayer: move " << moveDirectionLabel(dx, dy) << LOG_ENDL;

    if (destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;

    if (auto* door = game::findClosedDoorAt(*destMap, destLocal.x, destLocal.y, *database)) {
      door->tileId = door->tileId + 1;
      game::updateActiveMapVisibilityFromPlayer(world, avatar->x, avatar->y, *database);
      return;
    }

    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    // Town/outdoor: characters occupy tiles. Combat handles collide-to-attack separately.
    if (orch.findCharacterAt(destX, destY, avatar->id) != nullptr) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    avatar->x = destX;
    avatar->y = destY;
    game::queueStepTriggersAt(state->triggers, *destMap, destLocal.x, destLocal.y);
    game::updateActiveMapVisibilityFromPlayer(world, destX, destY, *database);
    if (!world.combat.active) {
      game::advanceWorldMovementTicks(*state, 1);
      world.resolvingTownEnemyAi = true;
      insertAction(new TownEnemyAiAfterPlayerMove(), 0);
    }
  }

public:
  WorldMovePlayer(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state

} // export
