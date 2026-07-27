#include "model/templates/CharacterTemplate.h"
#include "bmin/StringInterop.h"
#include <charconv>
#include <stdexcept>
#include <system_error>

namespace model {

namespace {

int parseSpriteOffsetIndex(const bmin::String& spriteOffset) {
  const auto view = bmin::toStringView(spriteOffset);
  int value = 0;
  const auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value);
  if (ec != std::errc{} || ptr != view.data() + view.size()) {
    throw std::runtime_error("Invalid character spriteOffset");
  }
  return value;
}

bmin::String buildCharacterSpriteName(const bmin::String& spritesheetName,
                                      const bmin::String& spriteOffset,
                                      int indexOffset) {
  const auto index = parseSpriteOffsetIndex(spriteOffset) + indexOffset;
  return spritesheetName + "_" + bmin::toString(index);
}

} // namespace

bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                               int indexOffset) {
  return buildCharacterSpriteName(
      characterTemplate.spritesheetName, characterTemplate.spriteOffset, indexOffset);
}

bmin::String characterGetSprite(const CharacterTemplate& characterTemplate) {
  return characterGetSpriteAtIndexOffset(characterTemplate, 0);
}

} // namespace model
