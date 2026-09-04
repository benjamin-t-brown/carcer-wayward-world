export module carcer.actions.ui.UiRemoveLayer;
export import carcer.state;
import bmin.string_interop;
import carcer.lib.StringUtil;

export {

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  bmin::String layerId;
  void act() override { LayerManagerInterface::closeLayer(bmin::toStringView(layerId)); }

public:
  UiRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  explicit UiRemoveLayer(LayerId id)
      : layerId(strutil::fromStringView(layerIdString(id))) {}
};

} // namespace actions

} // namespace state

} // export
