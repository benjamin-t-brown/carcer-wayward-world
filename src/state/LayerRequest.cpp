#include "state/LayerRequest.h"

#include "state/State.h"
#include <utility>

namespace state {

void pushLayerRequest(State& state, LayerRequest request) {
  for (auto it = state.uiState.layerStack.begin();
       it != state.uiState.layerStack.end();
       ++it) {
    if (it->id == request.id) {
      state.uiState.layerStack.erase(it);
      break;
    }
  }
  state.uiState.layerStack.pushBack(std::move(request));
}

void removeLayerRequest(State& state, LayerId id) {
  state.uiState.layerStack.eraseIf(
      [id](const LayerRequest& request) { return request.id == id; });
}

} // namespace state
