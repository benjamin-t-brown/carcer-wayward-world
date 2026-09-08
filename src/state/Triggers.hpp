#pragma once

#include "model/templates/Maps.h"
#include <optional>
#include "bmin/String.h"

namespace state {

struct Triggers {
  std::optional<bmin::String> pendingSpecialEventId;
  std::optional<model::TravelTrigger> pendingTravel;
  bool mapChangedThisTick = false;
};

} // namespace state
