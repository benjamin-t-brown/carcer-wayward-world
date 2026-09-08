#pragma once

#include "bmin/UniquePtr.h"
#include "model/templates/UtilityTypes.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/ActionEvent.hpp"
#include "state/StateManagerInterface.h"
#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace state {

struct State;

class AbstractAction : public state::DatabaseInterface,
                       public state::StateManagerInterface {
protected:
  State* state = nullptr;

  virtual void act() {
    sdl2w::Logger().get(sdl2w::WARN) << "AbstractAction::act() called noop";
  };

public:
  virtual ActionEvent getEvent() const { return ActionEvent::None; }
  virtual int getEventValue() const { return 0; }

  virtual bmin::String getName() const {
#ifdef __GNUG__
    int status;
    char* realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    bmin::String name = (status == 0) ? realname : typeid(*this).name();
    free(realname);
    return name;
#else
    return typeid(*this).name();
#endif
  }

  void setState(State* state) { this->state = state; }

  void execute(State* state) {
    this->state = state;
    // LOG(INFO) << "Executing action: " << getName() << LOG_ENDL;
    act();
  }

  void insertAction(bmin::UniquePtr<AbstractAction> action, int ms = 0) {
    auto* stateManager = getStateManager();
    if (stateManager == nullptr) {
      return; // action is destroyed here when there is no manager
    }
    stateManager->insertAction(bmin::move(action), ms);
  }

  void enqueueAction(bmin::UniquePtr<AbstractAction> action, int ms = 0) {
    auto* stateManager = getStateManager();
    if (stateManager == nullptr) {
      return; // action is destroyed here when there is no manager
    }
    stateManager->enqueueAction(bmin::move(action), ms);
  }

  // Schedule a pure delay: a null action advances the timer without running
  // anything. Distinct from an owning raw pointer; nothing is owned here.
  void insertAction(decltype(nullptr), int ms = 0) {
    insertAction(bmin::UniquePtr<AbstractAction>(), ms);
  }

  void enqueueAction(decltype(nullptr), int ms = 0) {
    enqueueAction(bmin::UniquePtr<AbstractAction>(), ms);
  }

  virtual ~AbstractAction() = default;
};

// Allocate a concrete action as an owning handle to its abstract base. This is
// the single allocation point for scheduled actions; bmin::UniquePtr has no
// derived-to-base converting constructor and the pinned dependency must not be
// modified, so callers cannot pass bmin::makeUnique<Derived>() directly.
template <typename T, typename... Args>
bmin::UniquePtr<AbstractAction> makeAction(Args&&... args) {
  return bmin::UniquePtr<AbstractAction>(new T(bmin::forward<Args>(args)...));
}

struct AsyncAction {
  bmin::UniquePtr<state::AbstractAction> action;
  model::TimerStruct timer;
};

} // namespace state
