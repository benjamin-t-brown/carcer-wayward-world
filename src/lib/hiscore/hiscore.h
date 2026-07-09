#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace hiscore {

struct HiscoreRow {
  bmin::String name;
  int score;
};

bmin::DynArray<HiscoreRow> getHighScores();

void saveHighScores(const bmin::DynArray<HiscoreRow>& hiscores);

}; // namespace hiscore
