#include "../setupTestState.hpp"
#include "db/Database.h"
#include "in3/EventRunnerHelpers.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"

namespace {

bool assertEqualStr(const bmin::String& actual, const char* expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual.cStr()
               << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertStorage(const bmin::Map<bmin::String, bmin::String>& storage,
                   const char* key,
                   const char* expected) {
  const auto value = in3::getStorage(storage, key);
  if (!value) {
    LOG(ERROR) << "missing storage key " << key << LOG_ENDL;
    return false;
  }
  return assertEqualStr(*value, expected, key);
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestLoadTestStateFixture" << LOG_ENDL;
  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    database.load();

    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();

    loadTestStateFromJson("__test__/ui/fixtures/layer-world.json", database, state);

    ok = assertStorage(state.specialEventStorage, "vars.hasSpokenToGateGuardJerry",
                       "true") &&
         ok;
    ok = assertStorage(state.specialEventStorage, "vars.aspect.aeon", "1") && ok;
    ok = assertStorage(state.specialEventStorage, "vars.aspect.resist", "1") && ok;
    ok = assertStorage(state.specialEventStorage,
                       "vars.quests.que_alinea_sealOfApproval.step", "step1") &&
         ok;
    ok = assertStorage(state.specialEventStorage, "vars.items.que_realmShedKey", "1") &&
         ok;
    ok = assertStorage(state.specialEventStorage, "once.l5fJ1BTynu", "true") && ok;
    ok = assertStorage(state.specialEventStorage, "tmp.UQhpQTTHLk", "true") && ok;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }

  if (!ok) {
    LOG(ERROR) << "TestLoadTestStateFixture assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestLoadTestStateFixture completed successfully" << LOG_ENDL;
  return 0;
}
