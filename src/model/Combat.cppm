module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.Combat;
export import bmin.containers;
import bmin.string_interop;
export import carcer.model.instances.CharacterInstance;
export import carcer.model.instances.Player;
import sdl2w;
#include "macros.h"

export {

// --- from model/Combat.h ---
namespace model {


inline constexpr int COMBAT_STARTING_AP = 4;
inline constexpr int COMBAT_MOVE_COST = 1;
inline constexpr int COMBAT_ATTACK_COST = 4;
inline constexpr int COMBAT_MELEE_DAMAGE = 10;
inline constexpr int COMBAT_HIT_CHANCE_PERCENT = 75;

enum class CombatActionType { MOVE, SHOOT, SPELL, WAIT };

struct SpellTargetInfo {
  bmin::String targetCharacterId;
  int tileX = 0;
  int tileY = 0;
};

struct Combat {
  bool active = false;
  bmin::DynArray<bmin::String> turnOrderIds;
  int activeTurnIndex = 0;
  bmin::String activeCharacterId;
  bool isWaitingForAction = false;
};

void removeCharacterFromCombatTurnOrder(Combat& combat, const bmin::String& characterId);

bool isPartyMember(const Player& player, const bmin::String& characterId);
bool isCharacterAlly(const Player& player, const CharacterInstance& character);
bool isCharacterEnemy(const CharacterInstance& character);

int getCharacterHp(const Player& player, const CharacterInstance& character);
void setCharacterHp(Player& player, CharacterInstance& character, int hp);
/** Updates party member HP by instance id without requiring them on the map. */
bool modifyPartyMemberHp(Player& player, const bmin::String& instanceId, int delta);
bool isCharacterDefeated(const Player& player, const CharacterInstance& character);

} // namespace model

} // export

namespace model {

bool isPartyMember(const Player& player, const bmin::String& characterId) {
  for (const auto& member : player.party) {
    if (member.instanceId == characterId) {
      return true;
    }
  }
  return false;
}

bool isCharacterEnemy(const CharacterInstance& character) {
  return characterInstanceIsEnemy(character);
}

bool isCharacterAlly(const Player& player, const CharacterInstance& character) {
  if (isPartyMember(player, character.id)) {
    return true;
  }
  return !isCharacterEnemy(character);
}

int getCharacterHp(const Player& player, const CharacterInstance& character) {
  if (isPartyMember(player, character.id)) {
    for (const auto& member : player.party) {
      if (member.instanceId == character.id) {
        return member.currentHp;
      }
    }
  }
  if (character.hpInitialized) {
    return character.currentHp;
  }
  if (character.currentHp > 0) {
    return character.currentHp;
  }
  return character.maxHp;
}

void setCharacterHp(Player& player, CharacterInstance& character, int hp) {
  if (isPartyMember(player, character.id)) {
    for (auto& member : player.party) {
      if (member.instanceId == character.id) {
        member.currentHp = hp;
        return;
      }
    }
  }
  character.currentHp = hp;
  character.hpInitialized = true;
}

bool modifyPartyMemberHp(Player& player, const bmin::String& instanceId, int delta) {
  for (auto& member : player.party) {
    if (member.instanceId == instanceId) {
      member.currentHp += delta;
      return true;
    }
  }
  return false;
}

bool isCharacterDefeated(const Player& player, const CharacterInstance& character) {
  return getCharacterHp(player, character) <= 0;
}

void removeCharacterFromCombatTurnOrder(Combat& combat, const bmin::String& characterId) {
  for (size_t i = 0; i < combat.turnOrderIds.size();) {
    if (combat.turnOrderIds[i] != characterId) {
      ++i;
      continue;
    }
    combat.turnOrderIds.erase(i);
    if (combat.activeTurnIndex > static_cast<int>(i)) {
      combat.activeTurnIndex -= 1;
    } else if (!combat.turnOrderIds.empty() &&
               combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
      combat.activeTurnIndex = 0;
    }
  }
  if (combat.activeCharacterId == characterId) {
    combat.activeCharacterId = bmin::String{};
  }
}

} // namespace model
