module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <utility>

module carcer.model;

import bmin.string_interop;

#include "macros.h"

// --- World.cppm ---

namespace model {

void resetAllCombatAp(World& world, int ap) {
  for (auto& character : world.activeMap.characters) {
    character.currentAp = ap;
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
