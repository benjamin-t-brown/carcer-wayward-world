#pragma once

// #include "state/AbstractAction.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include <memory>
#include <vector>

namespace db {
class Database;
}

namespace state {

class AbstractAction;
struct AsyncAction;

class StateManager : public state::DatabaseInterface {
private:
  state::State state;
  std::vector<std::unique_ptr<state::AsyncAction>> sequentialActions;
  std::vector<std::unique_ptr<state::AsyncAction>> sequentialActionsNext;
  std::vector<std::unique_ptr<state::AsyncAction>> parallelActions;

public:
  StateManager();
  ~StateManager() = default;

  state::State& getState();

  void enqueueAction(state::State& state, state::AbstractAction* action, int ms);

  void
  addParallelAction(State& state, std::unique_ptr<state::AbstractAction> action, int ms);

  void moveSequentialActions(State& state);

  void update(int dt);
};

} // namespace state