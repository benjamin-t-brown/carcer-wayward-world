#pragma once

#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"

namespace db {
class Database;
}

namespace state {
struct State;
}

namespace model {

struct MapInstance;
struct PersistentMapState;
struct Player;
struct World;
struct CharacterInstance;
struct ActiveMap;

inline constexpr int COMBAT_STARTING_AP = 4;
inline constexpr int COMBAT_MOVE_COST = 1;
inline constexpr int COMBAT_ATTACK_COST = 4;
inline constexpr int COMBAT_MELEE_DAMAGE = 10;
inline constexpr int COMBAT_HIT_CHANCE_PERCENT = 75;

enum class CombatActionType { MOVE, SHOOT, SPELL, WAIT };

struct CombatSpellTarget {
  bmin::String spellId;
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
bool isCharacterAlly(const Player& player,
                     const CharacterInstance& character,
                     const db::Database& database);
bool isCharacterEnemy(const CharacterInstance& character, const db::Database& database);

int getCharacterHp(const Player& player,
                   const CharacterInstance& character,
                   const db::Database& database);
void setCharacterHp(Player& player,
                    CharacterInstance& character,
                    int hp,
                    const db::Database& database);
bool isCharacterDefeated(const Player& player,
                         const CharacterInstance& character,
                         const db::Database& database);

void resetAllCombatAp(World& world, int ap = COMBAT_STARTING_AP);
void onNewCombatRound(state::State& state, const db::Database& database);
void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database);
void removeExtraPartyMembersFromMap(World& world, const Player& player);

Combat createCombatFromWorld(const World& world,
                             const Player& player,
                             const db::Database& database);

bmin::String formatCharacterLogLabel(const ActiveMap& activeMap, const bmin::String& id);

} // namespace model
