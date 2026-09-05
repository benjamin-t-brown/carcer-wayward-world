module;
#include <utility>

export module carcer.actions.general;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class PlaySound : public AbstractAction {
  bmin::String soundName;

  void act() override {
    if (!state) {
      return;
    }
    if (soundName.empty()) {
      return;
    }
    LOG(DEBUG) << "PlaySound: " << soundName << LOG_ENDL;
    if (state->soundsToPlay.contains(soundName)) {
      return;
    }
    state->soundsToPlay.pushBack(soundName);
  }

public:
  explicit PlaySound(bmin::String _soundName) : soundName(std::move(_soundName)) {}
};

} // namespace actions

} // namespace state

} // export
