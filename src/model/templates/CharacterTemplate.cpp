#include "model/templates/CharacterTemplate.h"

namespace model {

std::string characterGetSprite(const CharacterTemplate& characterTemplate) {
  return characterTemplate.spritesheetName + "_" + characterTemplate.spriteOffset;
}

} // namespace model
