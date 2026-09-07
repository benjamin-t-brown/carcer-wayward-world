#include "state/ActionBus.h"
#include "state/AbstractAction.h"

namespace state {

void ActionBus::subscribe(void* owner,
                           ActionEvent event,
                           std::function<void(AbstractAction&, State&)> handler) {
  entries.pushBack(Entry{
      .owner = owner,
      .event = event,
      .handler = std::move(handler),
  });
}

void ActionBus::unsubscribe(void* owner) {
  entries.eraseIf([owner](const Entry& entry) { return entry.owner == owner; });
}

void ActionBus::notify(AbstractAction& action, State& state) {
  const auto event = action.getEvent();
  for (const auto& entry : entries) {
    if (entry.event == event) {
      entry.handler(action, state);
    }
  }
}

} // namespace state
