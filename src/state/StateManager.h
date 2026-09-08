#pragma once

#include "bmin/DynArray.h"
#include "bmin/List.h"
#include "bmin/UniquePtr.h"
#include "state/ActionBus.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/UiStateUpdater.h"

namespace db {
class Database;
}

namespace state {

class AbstractAction;
struct AsyncAction;

struct ActionData {
  bmin::List<bmin::UniquePtr<AsyncAction>> sequentialActions;
  bmin::List<bmin::UniquePtr<AsyncAction>> sequentialActionsNext;
  bmin::List<bmin::UniquePtr<AsyncAction>> insertActions;
  bmin::DynArray<bmin::UniquePtr<AsyncAction>> parallelActions;
};

class StateManager : public state::DatabaseInterface {
private:
  state::State state;
  ActionData actionData;
  ActionBus actionBus;
  UiStateUpdater uiStateUpdater;

public:
  StateManager();
  ~StateManager() = default;

  state::State& getState();
  ActionData& getActionData();
  ActionBus& getActionBus();
  const ActionBus& getActionBus() const;

  void enqueueAction(ActionData& actions, AbstractAction* action, int ms);
  void insertAction(ActionData& actions, AbstractAction* action, int ms);
  void pllAction(ActionData& actions, AbstractAction* action, int ms);
  void moveSequentialActions(ActionData& actions);
  void moveInsertActions(ActionData& actions);

  void update(int dt);
};

} // namespace state
