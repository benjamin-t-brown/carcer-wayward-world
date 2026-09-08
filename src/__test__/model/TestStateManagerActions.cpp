#include "bmin/DynArray.h"
#include "actions/navigation/UiContinueSpecialEvent.hpp"
#include "actions/navigation/UiSelectSpecialEventChoice.hpp"
#include "actions/navigation/UiShowLayerPopupText.hpp"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
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
      stateManager->insertAction(bmin::UniquePtr<state::AbstractAction>(action), insertMs);
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

    sm.enqueueAction(state::makeAction<RecordAction>(&order, 1), 0);
    sm.enqueueAction(state::makeAction<RecordAction>(&order, 2), 0);
    sm.enqueueAction(state::makeAction<RecordAction>(&order, 3), 0);

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
    sm.enqueueAction(state::makeAction<RecordAction>(&order, 10, &sm, inserted, 0),
                     0);
    sm.enqueueAction(state::makeAction<RecordAction>(&order, 30), 0);

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

    sm.enqueueAction(state::makeAction<RecordAction>(&order, 1, &sm, std::move(children), 0),
                     0);
    sm.enqueueAction(state::makeAction<RecordAction>(&order, 99), 0);

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
    sm.enqueueAction(state::makeAction<RecordAction>(&order, 1, &sm, inserted, 0),
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

  // parallelAction: parallel queue executes when timer completes
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    sm.parallelAction(state::makeAction<RecordAction>(&order, 1), 0);
    sm.parallelAction(state::makeAction<RecordAction>(&order, 2), 0);

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

  // parallelAction with delay: runs only after timer
  {
    state::StateManager sm;
    bmin::DynArray<int> order;

    sm.parallelAction(state::makeAction<RecordAction>(&order, 7), 50);

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

    sm.enqueueAction(state::makeAction<RecordAction>(&order, 1), 0);
    sm.parallelAction(state::makeAction<RecordAction>(&order, 2), 0);

    sm.update(1);

    ok = assertOrder(order, {1, 2}, "enqueue then pll both run same update") && ok;
  }

  // Notification expiry is state maintenance, not a state-owned concrete action.
  {
    state::StateManager sm;
    auto& uiState = sm.getState().uiState;
    state::UiFloatingNotification notification;
    notification.id = "expired";
    model::timerStructStart(notification.timer, 10);
    uiState.floatingNotifications.pushBack(std::move(notification));

    sm.update(9);
    ok = assertEqual(static_cast<int>(uiState.floatingNotifications.size()),
                     1,
                     "notification remains before expiry") &&
         ok;
    ok = assertEqual(static_cast<int>(uiState.floatingNotificationRevision),
                     0,
                     "notification revision unchanged before expiry") &&
         ok;

    sm.update(1);
    ok = assertTrue(uiState.floatingNotifications.empty(),
                    "notification removed at expiry") &&
         ok;
    ok = assertEqual(static_cast<int>(uiState.floatingNotificationRevision),
                     1,
                     "notification expiry increments revision") &&
         ok;
  }

  // Subscribers receive stable semantic events and payloads without RTTI.
  {
    state::StateManager sm;
    int owner = 0;
    int continueCount = 0;
    int selectedChoice = -1;
    sm.getActionBus().subscribe(
        &owner,
        state::ActionEvent::UiContinueSpecialEvent,
        [&](state::AbstractAction&, state::State&) { ++continueCount; });
    sm.getActionBus().subscribe(
        &owner,
        state::ActionEvent::UiSelectSpecialEventChoice,
        [&](state::AbstractAction& action, state::State&) {
          selectedChoice = action.getEventValue();
        });

    sm.enqueueAction(state::makeAction<state::actions::UiContinueSpecialEvent>(),
                     0);
    sm.enqueueAction(state::makeAction<state::actions::UiSelectSpecialEventChoice>(3),
                     0);
    sm.update(1);

    ok = assertEqual(continueCount, 1, "semantic continue event delivered") && ok;
    ok = assertEqual(selectedChoice, 3, "semantic choice payload delivered") && ok;
    sm.getActionBus().unsubscribe(&owner);
  }

  // Navigation actions write neutral layer requests; they never construct layers.
  {
    state::StateManager sm;
    sm.enqueueAction(state::makeAction<state::actions::UiShowLayerPopupText>(
                         nullptr, "A title", "Some text"),
                     0);
    sm.update(1);

    const auto& requests = sm.getState().uiState.layerStack;
    ok = assertEqual(static_cast<int>(requests.size()),
                     1,
                     "navigation action writes one request") &&
         ok;
    if (!requests.empty()) {
      ok = assertTrue(requests[0].id == state::LayerId::PopupText,
                      "navigation request carries layer id") &&
           ok;
      ok = assertTrue(requests[0].a == "A title",
                      "navigation request carries title") &&
           ok;
      ok = assertTrue(requests[0].b == "Some text",
                      "navigation request carries text") &&
           ok;
    }
  }

  if (ok) {
    LOG(INFO) << "TestStateManagerActions passed" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestStateManagerActions failed" << LOG_ENDL;
  return 1;
}
