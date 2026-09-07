#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiRemoveLayer; }
  std::optional<LayerId> layerId;
  void act() override {
    if (!state) {
      return;
    }
    if (layerId) {
      removeLayerRequest(*state, *layerId);
    }
  }

public:
  explicit UiRemoveLayer(const bmin::String& value)
      : layerId(layerIdFromString(bmin::toStringView(value))) {}
  explicit UiRemoveLayer(LayerId value) : layerId(value) {}
};

} // namespace actions

} // namespace state
