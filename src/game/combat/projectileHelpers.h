#pragma once

#include "bmin/String.h"
#include "model/templates/AbilityTypes.h"

namespace game {

bmin::String getProjectileFacingSuffix(int dx, int dy);
int getProjectileTravelDurationMs(model::ProjectilePath path);

} // namespace game
