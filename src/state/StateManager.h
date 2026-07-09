#pragma once

#include "lib/Types.h"
#include "state/ActionBus.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/UiManager.h"

namespace db {
class Database;
}

namespace state {

class AbstractAction;
struct AsyncAction;

struct ActionData {
  List<UniquePtr<AsyncAction>> sequentialActions;
  List<UniquePtr<AsyncAction>> sequentialActionsNext;
  List<UniquePtr<AsyncAction>> insertActions;
  DynArray<UniquePtr<AsyncAction>> parallelActions;
};

class StateManager : public state::DatabaseInterface {
private:
  state::State state;
  ActionData actionData;
  ActionBus actionBus;
  UiManager uiManager;

public:
  StateManager();
  ~StateManager() = default;

  state::State& getState();
  ActionData& getActionData();
  ActionBus& getActionBus();
  const ActionBus& getActionBus() const;

  void enqueueAction(ActionData& actions, AbstractAction* action, int ms);
  void insertAction(ActionData& actions, AbstractAction* action, int ms);
  void addParallelAction(ActionData& actions, AbstractAction* action, int ms);
  void moveSequentialActions(ActionData& actions);

  void update(int dt);
};

} // namespace state
