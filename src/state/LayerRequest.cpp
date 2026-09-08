#include "state/LayerRequest.h"

#include "state/State.hpp"
#include <utility>

namespace state {

void pushLayerRequest(State& state, LayerRequest request) {
  state.uiState.layerCommands.pushBack(
      LayerCommand{LayerCommandType::Push, std::move(request)});
}

void removeLayerRequest(State& state, LayerId id) {
  state.uiState.layerCommands.pushBack(
      LayerCommand{LayerCommandType::Remove, LayerRequest{.id = id}});
}

} // namespace state
