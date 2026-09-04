module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.instances:Player;
export import bmin.containers;
import bmin.string_interop;
export import :CharacterPlayer;
import sdl2w;
#include "macros.h"

export {

// --- from model/instances/Player.h ---
namespace model {
struct Player {
  bmin::String name;
  bmin::DynArray<model::CharacterPlayer> party;
  int currentPartyMemberIndex = 0;
  int currentPartyMemberInventoryIndex = 0;
  int currentPartyMemberMagicIndex = 0;
  int gold = 0;
  int food = 0;
};
CharacterPlayer* playerFindPartyMemberById(Player& _player, const bmin::String& _id);
CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index);
/** Index of party member with instanceId, or -1 if not found. */
int playerFindPartyMemberIndexById(const Player& _player, const bmin::String& _id);

} // namespace model

} // export

namespace model {

CharacterPlayer* playerFindPartyMemberById(Player& _player, const bmin::String& _id) {
  for (auto& member : _player.party) {
    if (member.instanceId == _id) {
      return &member;
    }
  }
  return nullptr;
}

CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index) {
  if (_index < 0 || static_cast<size_t>(_index) >= _player.party.size()) {
    return nullptr;
  }
  return &_player.party[_index];
}

int playerFindPartyMemberIndexById(const Player& _player, const bmin::String& _id) {
  if (_id.empty()) {
    return -1;
  }
  for (int i = 0; i < static_cast<int>(_player.party.size()); i++) {
    if (_player.party[static_cast<size_t>(i)].instanceId == _id) {
      return i;
    }
  }
  return -1;
}

} // namespace model
