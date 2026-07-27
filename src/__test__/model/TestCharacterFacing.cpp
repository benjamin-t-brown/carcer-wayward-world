#include "model/instances/CharacterInstance.h"
#include "sdl2w/Logger.h"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertFacing(int dx, int dy, model::CharacterFacing expected, const char* label) {
  const auto facing = model::facingFromMoveDelta(dx, dy);
  if (facing != expected) {
    LOG(ERROR) << label << " expected "
               << (expected == model::CharacterFacing::Right ? "Right" : "Left")
               << " for (" << dx << ", " << dy << ")" << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestCharacterFacing" << LOG_ENDL;

  auto ok = true;
  ok = assertFacing(0, -1, model::CharacterFacing::Right, "up") && ok;
  ok = assertFacing(1, -1, model::CharacterFacing::Right, "up-right") && ok;
  ok = assertFacing(1, 0, model::CharacterFacing::Right, "right") && ok;
  ok = assertFacing(1, 1, model::CharacterFacing::Right, "down-right") && ok;
  ok = assertFacing(-1, 0, model::CharacterFacing::Left, "left") && ok;
  ok = assertFacing(-1, -1, model::CharacterFacing::Left, "up-left") && ok;
  ok = assertFacing(-1, 1, model::CharacterFacing::Left, "down-left") && ok;
  ok = assertFacing(0, 1, model::CharacterFacing::Left, "down") && ok;

  {
    auto character = model::CharacterInstance{};
    ok = assertTrue(character.facing == model::CharacterFacing::Right, "default facing right") &&
         ok;
    model::updateCharacterFacingFromMove(character, 0, 1);
    ok = assertTrue(character.facing == model::CharacterFacing::Left, "update facing from move") &&
         ok;
    model::updateCharacterFacingFromMove(character, 0, 0);
    ok = assertTrue(character.facing == model::CharacterFacing::Left, "zero delta keeps facing") &&
         ok;
  }

  {
    auto attacker = model::CharacterInstance{};
    attacker.x = 2;
    attacker.y = 2;
    attacker.facing = model::CharacterFacing::Left;
    model::updateCharacterFacingToward(attacker, 3, 2);
    ok = assertTrue(attacker.facing == model::CharacterFacing::Right, "face toward victim east") &&
         ok;
    model::updateCharacterFacingToward(attacker, 1, 4);
    ok = assertTrue(attacker.facing == model::CharacterFacing::Left, "face toward victim south") &&
         ok;
  }

  if (ok) {
    LOG(INFO) << "TestCharacterFacing PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestCharacterFacing FAILED" << LOG_ENDL;
  return 1;
}
