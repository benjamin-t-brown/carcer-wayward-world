#include "hiscore.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/sdl2w/Logger.h"
#include <sstream>

namespace hiscore {
bool isLoaded = false;
std::string hiscorePath = "hiscore.txt";
std::vector<HiscoreRow> hiscores;

std::vector<HiscoreRow> parseHiscoreText(const std::string& hiscoreText) {
  std::vector<HiscoreRow> hiscores;
  std::stringstream ss(hiscoreText);
  std::string line;
  while (std::getline(ss, line)) {
    std::stringstream lineStream(line);
    std::string name;
    int score;
    lineStream >> name >> score;
    LOG(INFO) << "Parsed hiscore: " << name << " " << score << LOG_ENDL;
    hiscores.push_back({name, score});
  }
  return hiscores;
}

std::string serializeHiscoreRow(const HiscoreRow& row) {
  std::stringstream ss;
  ss << row.name << " " << row.score;
  return ss.str();
}

std::vector<HiscoreRow> getHighScores() {
  if (!isLoaded) {
    try {
      const std::string hiscoreText = sdl2w::loadFileAsString(hiscorePath);
      if (hiscoreText.size() == 0) {
        saveHighScores(hiscores);
        isLoaded = true;
        return hiscores;
      }
      hiscores = parseHiscoreText(hiscoreText);
    } catch (std::exception& e) {
      // if file does not exist, create it
      saveHighScores(hiscores);
    }
    isLoaded = true;
  }
  return hiscores;
  return {};
}

void saveHighScores(const std::vector<HiscoreRow>& hiscoresA) {
  hiscores = hiscoresA;

  std::string content = "";
  for (auto& score : hiscoresA) {
    content += serializeHiscoreRow(score) + "\n";
  }
  sdl2w::saveFileAsString(hiscorePath, content);
}
} // namespace hiscore