#pragma once

#include "state/State.h"
#include <functional>
#include <typeindex>
#include <vector>

namespace state {

class AbstractAction;

class ActionBus {
  struct Entry {
    void* owner = nullptr;
    std::type_index actionType{typeid(void)};
    std::function<void(AbstractAction&, State&)> handler;
  };

  std::vector<Entry> entries;

public:
  void subscribe(void* owner,
                 std::type_index actionType,
                 std::function<void(AbstractAction&, State&)> handler);

  void unsubscribe(void* owner);
  void notify(AbstractAction& action, State& state);
};

} // namespace state
