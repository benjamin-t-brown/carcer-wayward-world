#pragma once

#include "state/AbstractAction.h"
#include <utility>

namespace state::actions {

/** Move-only owning handle used where an action must cross an API boundary. */
class Command {
  AbstractAction* action = nullptr;

public:
  explicit Command(AbstractAction* action) : action(action) {}
  Command(Command&& other) noexcept : action(std::exchange(other.action, nullptr)) {}
  Command& operator=(Command&& other) noexcept {
    if (this != &other) {
      delete action;
      action = std::exchange(other.action, nullptr);
    }
    return *this;
  }
  Command(const Command&) = delete;
  Command& operator=(const Command&) = delete;
  ~Command() { delete action; }

  void execute(State* state) {
    if (action) {
      action->execute(state);
    }
  }
  AbstractAction* release() { return std::exchange(action, nullptr); }
  operator AbstractAction*() && { return release(); }
};

} // namespace state::actions
