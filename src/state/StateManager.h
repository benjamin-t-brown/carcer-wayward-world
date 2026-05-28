#pragma once

#include "state/ActionBus.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/UiManager.h"
#include <list>
#include <memory>
#include <vector>

namespace db {
class Database;
}

namespace state {

class AbstractAction;
struct AsyncAction;

struct ActionData {
  std::list<std::unique_ptr<AsyncAction>> sequentialActions;
  std::list<std::unique_ptr<AsyncAction>> sequentialActionsNext;
  std::list<std::unique_ptr<AsyncAction>> insertActions;
  std::vector<std::unique_ptr<AsyncAction>> parallelActions;
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
