module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>

export module carcer.ui.ObserverRemoveLayer;
import carcer.actions.ui.UiRemoveLayer;
export import bmin.containers;
export import carcer.lib.StringUtil;
export import carcer.ui.UiElement;
import carcer.state;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

// --- from ui/observers/ObserverRemoveLayer.hpp ---
namespace ui {
class ObserverRemoveLayer : public ui::UiEventObserver,
                            public state::StateManagerInterface {
  bmin::String layerId;

public:
  ObserverRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  ObserverRemoveLayer(std::string_view _layerId) : layerId(strutil::fromStringView(_layerId)) {}
  explicit ObserverRemoveLayer(state::LayerId id)
      : layerId(strutil::fromStringView(state::layerIdString(id))) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverRemoveLayer::onClick " << layerId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || layerId.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiRemoveLayer(layerId), 0);
  }
};
} // namespace ui

} // export
