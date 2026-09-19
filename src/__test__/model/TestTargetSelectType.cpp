#include "model/templates/AbilityTypes.h"
#include "sdl2w/Logger.h"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestTargetSelectType" << LOG_ENDL;
  auto ok = true;

  ok = assertTrue(model::targetSelectTypeFromString("TARGET_ALLY") ==
                      model::TargetSelectType::TARGET_ALLY,
                  "parse TARGET_ALLY") &&
       ok;
  ok = assertTrue(model::targetSelectTypeToString(model::TargetSelectType::TARGET_ALLY) ==
                      "TARGET_ALLY",
                  "serialize TARGET_ALLY") &&
       ok;
  ok = assertTrue(model::targetSelectTypeFromString("TARGET_ZONE") ==
                      model::TargetSelectType::TARGET_ZONE,
                  "parse TARGET_ZONE") &&
       ok;

  if (!ok) {
    LOG(ERROR) << "TestTargetSelectType failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestTargetSelectType completed successfully" << LOG_ENDL;
  return 0;
}
