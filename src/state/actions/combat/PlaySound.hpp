#pragma once

#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class PlaySound : public CombatAction {
  bmin::String soundName;

  void act() override {
    LOG(DEBUG) << "PlaySound: " << soundName << LOG_ENDL;
  }

public:
  explicit PlaySound(bmin::String _soundName) : soundName(std::move(_soundName)) {}
};

} // namespace actions

} // namespace state
