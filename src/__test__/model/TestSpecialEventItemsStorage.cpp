#include "game/inventory/SpecialEventItemsStorage.h"
#include "in3/EventRunnerHelpers.h"
#include "sdl2w/Logger.h"

#define TEST_NAME "TestSpecialEventItemsStorage"

namespace {

bool assertOrder(const bmin::DynArray<bmin::String>& names,
                 std::initializer_list<const char*> expected) {
  if (names.size() != expected.size()) {
    LOG(ERROR) << "order size mismatch" << LOG_ENDL;
    return false;
  }
  size_t i = 0;
  for (const char* name : expected) {
    if (names[i] != bmin::String(name)) {
      LOG(ERROR) << "order mismatch at " << i << LOG_ENDL;
      return false;
    }
    ++i;
  }
  return true;
}

} // namespace

int main() {
  LOG(INFO) << "Starting " TEST_NAME << LOG_ENDL;
  bool ok = true;

  bmin::Map<bmin::String, bmin::String> storage;
  in3::setStorage(storage, "vars.items.alpha", "1");
  in3::setStorage(storage, "vars.items.beta", "1");
  in3::setStorage(storage, "vars.items.gamma", "1");

  bmin::DynArray<bmin::String> seededOrder;
  seededOrder.pushBack("gamma");
  seededOrder.pushBack("alpha");
  seededOrder.pushBack("beta");
  game::persistSpecialEventItemOrder(storage, seededOrder);

  auto initial = game::specialEventItemNamesInDisplayOrder(storage);
  ok = assertOrder(initial, {"gamma", "alpha", "beta"}) && ok;

  ok = game::reorderSpecialEventItemInStorage(storage, 1, -1) && ok;
  auto afterUp = game::specialEventItemNamesInDisplayOrder(storage);
  ok = assertOrder(afterUp, {"alpha", "gamma", "beta"}) && ok;

  ok = game::reorderSpecialEventItemInStorage(storage, 0, 1) && ok;
  auto afterDown = game::specialEventItemNamesInDisplayOrder(storage);
  ok = assertOrder(afterDown, {"gamma", "alpha", "beta"}) && ok;

  if (!ok) {
    return 1;
  }

  LOG(INFO) << TEST_NAME " passed" << LOG_ENDL;
  return 0;
}
