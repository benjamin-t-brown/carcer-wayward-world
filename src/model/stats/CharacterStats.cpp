#include "model/stats/CharacterStats.h"
#include "model/templates/CharacterTemplate.h"

namespace model {

void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl) {
  out = tmpl.stats;
}

} // namespace model
