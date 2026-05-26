#include "state/ActionBus.h"
#include "state/AbstractAction.h"
#include <algorithm>

namespace state {

void ActionBus::subscribe(void* owner,
                           std::type_index actionType,
                           std::function<void(AbstractAction&, State&)> handler) {
  entries.push_back(Entry{
      .owner = owner,
      .actionType = actionType,
      .handler = std::move(handler),
  });
}

void ActionBus::unsubscribe(void* owner) {
  entries.erase(
      std::remove_if(entries.begin(),
                     entries.end(),
                     [owner](const Entry& entry) { return entry.owner == owner; }),
      entries.end());
}

void ActionBus::notify(AbstractAction& action, State& state) {
  const std::type_index actionType = std::type_index(typeid(action));
  for (const auto& entry : entries) {
    if (entry.actionType == actionType) {
      entry.handler(action, state);
    }
  }
}

} // namespace state
