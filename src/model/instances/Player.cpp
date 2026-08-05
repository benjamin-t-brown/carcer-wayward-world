#include "model/instances/Player.h"

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
