#include "model/instances/Player.h"
#include <string>

namespace model {

CharacterPlayer* playerFindPartyMemberById(Player& _player, const std::string& _id) {
  for (auto& member : _player.party) {
    if (member.instanceId == _id) {
      return &member;
    }
  }
  return nullptr;
}

CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index) {
  if (_index < 0 || _index >= _player.party.size()) {
    return nullptr;
  }
  return &_player.party[_index];
}

} // namespace model
