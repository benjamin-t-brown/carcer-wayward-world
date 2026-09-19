#include "model/ColorHex.h"
#include "sdl2w/Logger.h"

namespace {

bool assertRgb(model::RgbColor actual, int r, int g, int b, const char* label) {
  if (actual.r != r || actual.g != g || actual.b != b) {
    LOG(ERROR) << label << " expected " << r << "," << g << "," << b << " but got "
               << static_cast<int>(actual.r) << "," << static_cast<int>(actual.g)
               << "," << static_cast<int>(actual.b) << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestColorHex" << LOG_ENDL;
  auto ok = true;

  ok = assertRgb(model::rgbColorFromHex("#FFFFFF"), 255, 255, 255, "white") && ok;
  ok = assertRgb(model::rgbColorFromHex(""), 255, 255, 255, "empty is white") && ok;
  ok = assertRgb(model::rgbColorFromHex("nope"), 255, 255, 255, "invalid is white") &&
       ok;
  ok = assertRgb(model::rgbColorFromHex("#111111"), 17, 17, 17, "singe dark") && ok;
  ok = assertRgb(model::rgbColorFromHex("FF0000"), 255, 0, 0, "red without hash") && ok;
  ok = assertRgb(model::rgbColorFromHex("#0F0"), 0, 255, 0, "short green") && ok;
  ok = assertRgb(model::rgbColorFromHex("#abc"), 170, 187, 204, "short mixed") && ok;

  if (!ok) {
    LOG(ERROR) << "TestColorHex failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestColorHex completed successfully" << LOG_ENDL;
  return 0;
}
