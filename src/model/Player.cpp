#include "Player.h"
#include <string>

namespace model {

CharacterPlayer* Player::findPartyMember(const std::string& _id) {
  for (auto& member : party) {
    if (member.id == _id) {
      return &member;
    }
  }
  return nullptr;
}

} // namespace model
