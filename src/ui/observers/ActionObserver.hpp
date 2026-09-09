#pragma once

#include "bmin/UniquePtr.h"
#include "state/AbstractAction.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "ui/UiElement.h"
#include <functional>
#include <utility>

namespace ui {

// Reusable click observer. Owns a factory that, on each click, builds a fresh
// owning action (or an empty handle to skip) and enqueues it through the state
// manager. This replaces the family of one-method observer classes whose only
// behavior was to store constructor arguments and forward a single action.
//
// The factory receives the live StateManager so callers can read current state
// at click time (e.g. gate on combat, compute an index, read a popup value) and
// return an empty handle to do nothing.
class ActionObserver : public ui::UiEventObserver,
                       public state::StateManagerInterface {
public:
  using Factory =
      std::function<bmin::UniquePtr<state::AbstractAction>(state::StateManager&)>;

private:
  Factory factory;

public:
  explicit ActionObserver(Factory _factory) : factory(std::move(_factory)) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    auto* stateManager = getStateManager();
    if (!stateManager || !factory) {
      return;
    }
    if (auto action = factory(*stateManager)) {
      stateManager->enqueueAction(std::move(action), 0);
    }
  }
};

// Convenience binding for the common case: construct a fresh
// state::actions::T(args...) on every click. Repeated clicks build distinct
// action objects because the arguments are captured by value and the action is
// constructed inside the factory each time.
template <typename T, typename... Args>
bmin::UniquePtr<ui::UiEventObserver> makeActionObserver(Args... args) {
  return bmin::UniquePtr<ui::UiEventObserver>(
      new ActionObserver([args...](state::StateManager&) {
        return state::makeAction<T>(args...);
      }));
}

} // namespace ui
