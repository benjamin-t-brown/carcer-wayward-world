#pragma once

#include "lib/sdl2w/Logger.h"
#include "model/UtilityTypes.h"
#include "state/DatabaseInterface.h"
#include "state/LayerManagerInterface.h"
#include <memory>
#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace state {

struct State;

class AbstractAction : public state::DatabaseInterface,
                       public state::LayerManagerInterface {
protected:
  State* state = nullptr;

  virtual void act() {
    sdl2w::Logger().get(sdl2w::WARN) << "AbstractAction::act() called noop";
  };

public:
  virtual std::string getName() const {
#ifdef __GNUG__
    int status;
    char* realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    std::string name = (status == 0) ? realname : typeid(*this).name();
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

  virtual ~AbstractAction() = default;
};

struct AsyncAction {
  std::unique_ptr<state::AbstractAction> action;
  model::TimerStruct timer;
};

} // namespace state