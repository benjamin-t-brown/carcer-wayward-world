#pragma once

#include "model/Player.h"

namespace state {

struct UserSettings {
  int fontScale = 0;
};

struct State {
  model::Player player;
  UserSettings settings;
};
} // namespace state`