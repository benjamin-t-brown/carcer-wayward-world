#pragma once

#include <string>
#include <vector>

namespace hiscore {

struct HiscoreRow {
  std::string name;
  int score;
};

std::vector<HiscoreRow> getHighScores();

void saveHighScores(const std::vector<HiscoreRow>& hiscores);

}; // namespace hiscore