#include "model/templates/CharacterTemplate.h"

namespace model {

bmin::String characterGetSprite(const CharacterTemplate& characterTemplate) {
  return characterTemplate.spritesheetName + "_" + characterTemplate.spriteOffset;
}

} // namespace model
