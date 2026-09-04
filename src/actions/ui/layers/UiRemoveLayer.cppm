module;
#include <cstddef>

export module carcer.actions.ui.layers:UiRemoveLayer;
export import carcer.state;
import carcer.lib.StringUtil;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  bmin::String layerId;
  void act() override {
    if (!state) {
      return;
    }
    if (auto id = layerIdFromString(bmin::toStringView(layerId))) {
      removeLayerRequest(*state, *id);
    }
  }

public:
  UiRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  explicit UiRemoveLayer(LayerId id)
      : layerId(strutil::fromStringView(layerIdString(id))) {}
};

} // namespace actions

} // namespace state

} // export
