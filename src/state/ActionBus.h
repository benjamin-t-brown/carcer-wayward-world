#pragma once

#include "bmin/DynArray.h"
#include "state/ActionEvent.hpp"
#include "state/State.hpp"
#include <functional>

namespace state {

class AbstractAction;

class ActionBus {
  struct Entry {
    void* owner = nullptr;
    ActionEvent event = ActionEvent::None;
    std::function<void(AbstractAction&, State&)> handler;
  };

  bmin::DynArray<Entry> entries;

public:
  void subscribe(void* owner,
                 ActionEvent event,
                 std::function<void(AbstractAction&, State&)> handler);

  void unsubscribe(void* owner);
  void notify(AbstractAction& action, State& state);
};

} // namespace state
