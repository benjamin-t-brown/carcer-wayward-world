module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.lib.hiscore.hiscore;
export import bmin.containers;
import bmin.string_interop;

export {

// --- from lib/hiscore/hiscore.h ---
namespace hiscore {

struct HiscoreRow {
  bmin::String name;
  int score;
};

bmin::DynArray<HiscoreRow> getHighScores();

void saveHighScores(const bmin::DynArray<HiscoreRow>& hiscores);

}; // namespace hiscore

} // export
