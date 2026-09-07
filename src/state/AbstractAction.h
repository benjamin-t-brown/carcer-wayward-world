#pragma once

#include "bmin/UniquePtr.h"
#include "model/templates/UtilityTypes.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/ActionEvent.h"
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

  void insertAction(AbstractAction* action, int ms = 0) {
    auto* stateManager = getStateManager();
    if (stateManager == nullptr) {
      return;
    }
    stateManager->insertAction(stateManager->getActionData(), action, ms);
  }

  void enqueueAction(AbstractAction* action, int ms = 0) {
    auto* stateManager = getStateManager();
    if (stateManager == nullptr) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(), action, ms);
  }

  virtual ~AbstractAction() = default;
};

struct AsyncAction {
  bmin::UniquePtr<state::AbstractAction> action;
  model::TimerStruct timer;
};

} // namespace state
