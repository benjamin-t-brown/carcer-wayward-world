#pragma once

#include "lib/Types.h"

namespace hiscore {

struct HiscoreRow {
  String name;
  int score;
};

DynArray<HiscoreRow> getHighScores();

void saveHighScores(const DynArray<HiscoreRow>& hiscores);

}; // namespace hiscore
