module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#include <algorithm>

export module carcer.model.instances:World;
export import bmin.containers;
import bmin.string_interop;
export import carcer.db;
export import :Combat;
export import :MapInstance;
export import :Player;
export import carcer.model.templates;
import sdl2w;
import :CharacterInstance;
#include "macros.h"

export {

// --- from model/instances/World.h ---
namespace model {

enum class CameraMode { Follow, Aiming, Dragging, Controlled };

enum class WorldActionMode { NONE, EXAMINE, TALK, SPELL };

  // Map-space hit feedback: splash animation plus a numeric label (not UI floating text).
struct DamageParticle {
  bmin::String animationName;
  bmin::String text;
  int tileX = 0;
  int tileY = 0;
  TimerStruct lifetime;
};

// Traveling combat projectile (tile-space lerp from caster to zone origin).
struct WorldProjectile {
  bmin::String animationName;
  bmin::String text;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  ProjectilePath projectilePath = ProjectilePath::PROJECTILE_PATH_NONE;
  TimerStruct travel;
  int yOffset = 0;
};

struct CameraInfo {
  int camX = 0; // map pixel space
  int camY = 0;
  CameraMode cameraMode = CameraMode::Follow;
  // empty = auto-resolve to current party member avatar when cameraMode is Follow
  bmin::String cameraFollowCharacterId;
  int viewW = 0; // MapView content size in map-pixel space (unscaled)
  int viewH = 0;
};

struct ActiveMap {
  bmin::String gridId;
  int mapLayer = 0;
  bmin::DynArray<CharacterInstance> characters;
  bmin::DynArray<ItemInstance> items;
  // bmin::DynArray<TileField> fields;
  bmin::DynArray<DamageParticle> damageParticles;
  bmin::DynArray<WorldProjectile> projectiles;
};

struct World {
  ActiveMap activeMap;

  CameraInfo camera;
  WorldActionMode actionMode = WorldActionMode::NONE;
  // Meaningful only when actionMode != NONE (Examine / Talk / Spell aim cursor).
  std::optional<TileXY> actionAimTile;
  // Meaningful only when actionMode == SPELL (spell template name, e.g. "FLAME").
  bmin::String pendingSpellId;
  bmin::String pendingChId;

  // True while town enemy AI is resolving (seek / melee swing / particles).
  // Blocks player movement and world actions until the timed sequence finishes.
  bool resolvingTownEnemyAi = false;

  Combat combat;
};

void resetAllCombatAp(World& world, int ap = COMBAT_STARTING_AP);
void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database);
void removeExtraPartyMembersFromMap(World& world, const Player& player);

Combat createCombatFromWorld(const World& world, const Player& player);

bmin::String formatCharacterLogLabel(const ActiveMap& activeMap, const bmin::String& id);

} // namespace model

} // export

namespace model {

void resetAllCombatAp(World& world, int ap) {
  for (auto& character : world.activeMap.characters) {
    character.currentAp = ap;
  }
}

void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database) {
  auto& activeMap = world.activeMap;
  CharacterInstance* leader = nullptr;
  if (!player.party.empty()) {
    const auto& leaderId = player.party[0].instanceId;
    for (auto& character : activeMap.characters) {
      if (character.id == leaderId) {
        leader = &character;
        break;
      }
    }
  }
  const auto spawnX = leader ? leader->x : 0;
  const auto spawnY = leader ? leader->y : 0;

  for (const auto& member : player.party) {
    bool found = false;
    for (const auto& character : activeMap.characters) {
      if (character.id == member.instanceId) {
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }

    auto instance = CharacterInstance{};
    instance.id = member.instanceId;
    instance.name = member.name.empty() ? member.params.name : member.name;
    instance.templateName =
        member.templateName.empty() ? member.params.name : member.templateName;
    instance.x = spawnX;
    instance.y = spawnY;
    instance.spawnX = spawnX;
    instance.spawnY = spawnY;
    instance.currentAp = COMBAT_STARTING_AP;
    instance.currentHp = member.currentHp;
    tryApplyCharacterTemplateToInstance(instance, database);
    activeMap.characters.pushBack(std::move(instance));
  }

  for (auto& character : activeMap.characters) {
    if (character.currentHp <= 0 && isCharacterEnemy(character)) {
      character.currentHp = character.maxHp;
    }
  }
}

void removeExtraPartyMembersFromMap(World& world, const Player& player) {
  if (player.party.empty()) {
    return;
  }
  const auto& keepId = player.party[0].instanceId;
  auto& characters = world.activeMap.characters;
  for (size_t i = 0; i < characters.size();) {
    const auto& character = characters[i];
    if (isPartyMember(player, character.id) && character.id != keepId) {
      characters.erase(static_cast<size_t>(i));
      continue;
    }
    i++;
  }
}

Combat createCombatFromWorld(const World& world, const Player& player) {
  Combat combat;
  combat.active = true;
  combat.activeTurnIndex = 0;

  auto isInTurnOrder = [&](const bmin::String& id) {
    for (const auto& existingId : combat.turnOrderIds) {
      if (existingId == id) {
        return true;
      }
    }
    return false;
  };

  auto findOnActiveMap = [&](const bmin::String& id) {
    for (const auto& character : world.activeMap.characters) {
      if (character.id == id) {
        return true;
      }
    }
    return false;
  };

  for (const auto& member : player.party) {
    if (findOnActiveMap(member.instanceId) && !isInTurnOrder(member.instanceId)) {
      combat.turnOrderIds.pushBack(member.instanceId);
    }
  }

  for (const auto& character : world.activeMap.characters) {
    if (isInTurnOrder(character.id) || isCharacterEnemy(character)) {
      continue;
    }
    combat.turnOrderIds.pushBack(character.id);
  }

  for (const auto& character : world.activeMap.characters) {
    if (isInTurnOrder(character.id)) {
      continue;
    }
    combat.turnOrderIds.pushBack(character.id);
  }

  return combat;
}

bmin::String formatCharacterLogLabel(const ActiveMap& activeMap, const bmin::String& id) {
  const CharacterInstance* character = nullptr;
  for (const auto& ch : activeMap.characters) {
    if (ch.id == id) {
      character = &ch;
      break;
    }
  }
  if (character == nullptr || character->name.empty()) {
    return id;
  }
  bmin::StringStream ss;
  ss << character->name << " (" << id << ")";
  return bmin::String(ss.str().cStr());
}

} // namespace model
