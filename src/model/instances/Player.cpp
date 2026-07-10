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

} // namespace model
