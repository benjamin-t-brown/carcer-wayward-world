#include "bmin/DynArray.h"
#include "bmin/List.h"
#include "bmin/UniquePtr.h"
#include "lib/sdl2w/Logger.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestBminContainers" << LOG_ENDL;

  bool ok = true;

  bmin::DynArray<int> values;
  values.pushBack(1);
  values.pushBack(2);
  values.pushBack(3);
  values.resize(5);
  ok = assertEqual(static_cast<int>(values.size()), 5, "resize.grow") && ok;
  ok = assertEqual(values[0], 1, "resize.preserve[0]") && ok;
  ok = assertEqual(values[4], 0, "resize.default[4]") && ok;

  values.resize(2);
  ok = assertEqual(static_cast<int>(values.size()), 2, "resize.shrink") && ok;
  ok = assertEqual(values[1], 2, "resize.shrink[1]") && ok;

  values.insert(values.begin(), 99);
  ok = assertEqual(values[0], 99, "insert.begin") && ok;
  ok = assertEqual(static_cast<int>(values.size()), 3, "insert.size") && ok;

  values.erase(values.begin() + 1);
  ok = assertEqual(values[0], 99, "erase.middle[0]") && ok;
  ok = assertEqual(values[1], 2, "erase.middle[1]") && ok;

  values.erase(static_cast<size_t>(0));
  ok = assertEqual(values[0], 2, "erase.size_t") && ok;
  ok = assertEqual(static_cast<int>(values.size()), 1, "erase.size_t.size") && ok;

  values.pushBack(4);
  values.pushBack(5);
  const size_t removed = values.eraseIf([](const int& v) { return v % 2 == 0; });
  ok = assertEqual(static_cast<int>(removed), 2, "eraseIf.removed") && ok;
  ok = assertEqual(static_cast<int>(values.size()), 1, "eraseIf.size") && ok;
  ok = assertEqual(values[0], 5, "eraseIf[0]") && ok;

  values.pushBack(3);
  int reverseSum = 0;
  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    reverseSum += *it;
  }
  ok = assertEqual(reverseSum, 8, "rbegin.sum") && ok;

  bmin::List<int> queue;
  queue.pushBack(10);
  queue.pushBack(20);
  ok = assertEqual(queue.front(), 10, "list.front") && ok;

  bmin::List<int> pending;
  pending.pushBack(30);
  pending.pushBack(40);
  queue.splice(queue.end(), pending);
  ok = assertEqual(static_cast<int>(queue.size()), 4, "splice.size") && ok;
  ok = assertEqual(pending.empty(), true, "splice.clears_other") && ok;
  ok = assertEqual(queue.front(), 10, "splice.front") && ok;

  int listSum = 0;
  for (int v : queue) {
    listSum += v;
  }
  ok = assertEqual(listSum, 100, "splice.order") && ok;

  auto unique = bmin::makeUnique<int>(7);
  ok = assertEqual(*unique, 7, "makeUnique.value") && ok;

  if (!ok) {
    return 1;
  }

  LOG(INFO) << "TestBminContainers passed" << LOG_ENDL;
  return 0;
}
