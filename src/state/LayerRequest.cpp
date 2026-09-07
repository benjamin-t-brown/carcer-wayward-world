#include "state/LayerRequest.h"

#include "state/State.h"
#include <utility>

namespace state {

void pushLayerRequest(State& state, LayerRequest request) {
  for (auto& existing : state.uiState.layerStack) {
    if (existing.id == request.id) {
      existing = std::move(request);
      return;
    }
  }
  state.uiState.layerStack.pushBack(std::move(request));
}

void removeLayerRequest(State& state, LayerId id) {
  state.uiState.layerStack.eraseIf(
      [id](const LayerRequest& request) { return request.id == id; });
}

} // namespace state
