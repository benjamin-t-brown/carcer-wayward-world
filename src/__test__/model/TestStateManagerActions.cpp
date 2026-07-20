#include "bmin/DynArray.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/StateManager.h"

#include <initializer_list>

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertOrder(const bmin::DynArray<int>& actual,
                 std::initializer_list<int> expected,
                 const char* label) {
  if (actual.size() != expected.size()) {
    LOG(ERROR) << label << " size expected " << expected.size() << " but got "
               << actual.size() << LOG_ENDL;
    return false;
  }
  size_t i = 0;
  for (int value : expected) {
    if (actual[i] != value) {
      LOG(ERROR) << label << " at " << i << " expected " << value << " but got "
                 << actual[i] << LOG_ENDL;
      return false;
    }
    i++;
  }
  return true;
}

class RecordAction : public state::AbstractAction {
  bmin::DynArray<int>* order = nullptr;
  int id = 0;
  state::StateManager* stateManager = nullptr;
  bmin::DynArray<state::AbstractAction*> inserts;
  int insertMs = 0;

public:
  RecordAction(bmin::DynArray<int>* _order, int _id) : order(_order), id(_id) {}

  RecordAction(bmin::DynArray<int>* _order,
               int _id,
               state::StateManager* _stateManager,
               state::AbstractAction* _insertDuringAct,
               int _insertMs)
      : order(_order), id(_id), stateManager(_stateManager), insertMs(_insertMs) {
    inserts.pushBack(_insertDuringAct);
  }

  RecordAction(bmin::DynArray<int>* _order,
               int _id,
               state::StateManager* _stateManager,
               bmin::DynArray<state::AbstractAction*> _inserts,
               int _insertMs)
      : order(_order),
        id(_id),
        stateManager(_stateManager),
        inserts(std::move(_inserts)),
        insertMs(_insertMs) {}

  void act() override {
    if (order != nullptr) {
      order->pushBack(id);
    }
    if (stateManager == nullptr) {
      return;
    }
    for (auto* action : inserts) {
      stateManager->insertAction(stateManager->getActionData(), action, insertMs);
    }
    inserts.clear();
  }
};

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestStateManagerActions" << LOG_ENDL;
  auto ok = true;

  // enqueueAction: FIFO sequential execution with duration 0
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    sm.enqueueAction(sm.getActionData(), new RecordAction(&order, 1), 0);
    sm.enqueueAction(sm.getActionData(), new RecordAction(&order, 2), 0);
    sm.enqueueAction(sm.getActionData(), new RecordAction(&order, 3), 0);

    ok = assertEqual(static_cast<int>(sm.getActionData().sequentialActionsNext.size()),
                     3,
                     "enqueue fills sequentialActionsNext") &&
         ok;
    ok = assertTrue(sm.getActionData().sequentialActions.empty(),
                    "enqueue does not touch sequentialActions yet") &&
         ok;

    sm.update(1);

    ok = assertOrder(order, {1, 2, 3}, "enqueue executes FIFO") && ok;
    ok = assertTrue(sm.getActionData().sequentialActions.empty(),
                    "enqueue queue drained after duration 0") &&
         ok;
    ok = assertTrue(sm.getActionData().sequentialActionsNext.empty(),
                    "enqueue next queue drained") &&
         ok;
  }

  // insertAction: spliced immediately after the currently executing action
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    auto* inserted = new RecordAction(&order, 20);
    sm.enqueueAction(sm.getActionData(),
                     new RecordAction(&order, 10, &sm, inserted, 0),
                     0);
    sm.enqueueAction(sm.getActionData(), new RecordAction(&order, 30), 0);

    sm.update(1);

    ok = assertOrder(order, {10, 20, 30}, "insert runs after current, before later enqueue") &&
         ok;
    ok = assertTrue(sm.getActionData().insertActions.empty(),
                    "insertActions drained after splice") &&
         ok;
  }

  // Nested inserts: root inserts 3 children; each child inserts one grandchild.
  // Inserts splice after the currently executing action, so order is depth-first
  // per child: root, then child1+its insert, child2+its insert, child3+its insert.
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    auto* child1 = new RecordAction(&order, 10, &sm, new RecordAction(&order, 11), 0);
    auto* child2 = new RecordAction(&order, 20, &sm, new RecordAction(&order, 21), 0);
    auto* child3 = new RecordAction(&order, 30, &sm, new RecordAction(&order, 31), 0);
    bmin::DynArray<state::AbstractAction*> children{child1, child2, child3};

    sm.enqueueAction(sm.getActionData(),
                     new RecordAction(&order, 1, &sm, std::move(children), 0),
                     0);
    sm.enqueueAction(sm.getActionData(), new RecordAction(&order, 99), 0);

    sm.update(1);

    ok = assertOrder(order,
                     {1, 10, 11, 20, 21, 30, 31, 99},
                     "nested inserts run depth-first before later enqueue") &&
         ok;
    ok = assertTrue(sm.getActionData().insertActions.empty(),
                    "nested insertActions drained") &&
         ok;
    ok = assertTrue(sm.getActionData().sequentialActions.empty(),
                    "nested sequential queue drained") &&
         ok;
  }

  // insertAction behind a delayed front: next action waits for a later update
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    auto* inserted = new RecordAction(&order, 2);
    sm.enqueueAction(sm.getActionData(),
                     new RecordAction(&order, 1, &sm, inserted, 0),
                     100);

    sm.update(1);
    ok = assertOrder(order, {1}, "delayed front executes immediately") && ok;
    ok = assertEqual(static_cast<int>(sm.getActionData().sequentialActions.size()),
                     2,
                     "insert spliced after delayed front") &&
         ok;
    ok = assertEqual(static_cast<int>(order.size()), 1, "inserted not run while front waits") &&
         ok;

    sm.update(99);
    ok = assertEqual(static_cast<int>(order.size()), 1,
                     "non-zero duration does not continue same frame") &&
         ok;

    sm.update(1);
    ok = assertOrder(order, {1, 2}, "inserted runs on following update") && ok;
  }

  // pllAction: parallel queue executes when timer completes
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    sm.pllAction(sm.getActionData(), new RecordAction(&order, 1), 0);
    sm.pllAction(sm.getActionData(), new RecordAction(&order, 2), 0);

    ok = assertEqual(static_cast<int>(sm.getActionData().parallelActions.size()),
                     2,
                     "pll fills parallelActions") &&
         ok;

    sm.update(1);

    ok = assertOrder(order, {1, 2}, "pll executes both at duration 0") && ok;
    ok = assertEqual(static_cast<int>(sm.getActionData().parallelActions.size()),
                     0,
                     "pll queue drained") &&
         ok;
  }

  // pllAction with delay: runs only after timer
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    sm.pllAction(sm.getActionData(), new RecordAction(&order, 7), 50);

    sm.update(25);
    ok = assertEqual(static_cast<int>(order.size()), 0, "pll waits for timer") && ok;
    ok = assertEqual(static_cast<int>(sm.getActionData().parallelActions.size()),
                     1,
                     "pll still queued mid-timer") &&
         ok;

    sm.update(25);
    ok = assertOrder(order, {7}, "pll executes when timer completes") && ok;
    ok = assertEqual(static_cast<int>(sm.getActionData().parallelActions.size()),
                     0,
                     "pll drained after fire") &&
         ok;
  }

  // enqueue + pll in the same update both run
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    sm.enqueueAction(sm.getActionData(), new RecordAction(&order, 1), 0);
    sm.pllAction(sm.getActionData(), new RecordAction(&order, 2), 0);

    sm.update(1);

    ok = assertOrder(order, {1, 2}, "enqueue then pll both run same update") && ok;
  }

  if (ok) {
    LOG(INFO) << "TestStateManagerActions passed" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestStateManagerActions failed" << LOG_ENDL;
  return 1;
}
