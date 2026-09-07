module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <stdexcept>

module carcer.data;
import carcer.lib.StringUtil;
import bmin.containers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace hiscore {
bool isLoaded = false;
bmin::String hiscorePath = "hiscore.txt";
bmin::DynArray<HiscoreRow> hiscores;

bmin::DynArray<HiscoreRow> parseHiscoreText(const bmin::String& hiscoreText) {
  bmin::DynArray<HiscoreRow> rows;
  const bmin::DynArray<bmin::String> lines = strutil::splitLines(hiscoreText);
  for (size_t i = 0; i < lines.size(); ++i) {
    bmin::String name;
    int score = 0;
    if (!strutil::parseFirstTokenAndInt(lines[i], name, score)) {
      continue;
    }
    LOG(INFO) << "Parsed hiscore: " << name << " " << score << LOG_ENDL;
    rows.pushBack({name, score});
  }
  return rows;
}

bmin::String serializeHiscoreRow(const HiscoreRow& row) {
  bmin::StringStream ss;
  ss << row.name << " " << row.score;
  return ss.str();
}

bmin::DynArray<HiscoreRow> getHighScores() {
  if (!isLoaded) {
    try {
      const bmin::String hiscoreText = sdl2w::loadFileAsString(hiscorePath.sliceView());
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

void saveHighScores(const bmin::DynArray<HiscoreRow>& hiscoresA) {
  hiscores = hiscoresA;

  bmin::String content;
  for (const auto& score : hiscoresA) {
    content += serializeHiscoreRow(score) + "\n";
  }
  sdl2w::saveFileAsString(hiscorePath.sliceView(), content.sliceView());
}
} // namespace hiscore
