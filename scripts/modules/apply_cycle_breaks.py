#!/usr/bin/env python3
"""Apply module-migration cycle breaks to classic headers (run after git checkout of headers)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"


def write(rel: str, text: str) -> None:
    p = SRC / rel
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(text.replace("\r\n", "\n"), encoding="utf-8", newline="\n")


def main() -> None:
    # StateManagerInterface: incomplete StateManager only
    write(
        "state/StateManagerInterface.h",
        """#pragma once

namespace state {
class StateManager;

class StateManagerInterface {
private:
  static state::StateManager* stateManager;

protected:
  static state::StateManager* getStateManager(bool throwIfNotSet = false);
  bool hasStateManager() const { return stateManager != nullptr; }

public:
  static void setStateManager(state::StateManager* _stateManager);
};

} // namespace state
""",
    )

    # LayerManagerInterface already fwds only — OK

    # CharacterStats: drop CharacterTemplate API
    p = SRC / "model/stats/CharacterStats.h"
    t = p.read_text(encoding="utf-8")
    t = t.replace(
        """
struct CharacterTemplate;

void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl);

} // namespace model
""",
        "\n} // namespace model\n",
    )
    write("model/stats/CharacterStats.h", t)

    # CharacterTemplate: drop Instance/Player/Database APIs; add initCharacterStatsFromTemplate
    p = SRC / "model/templates/CharacterTemplate.h"
    t = p.read_text(encoding="utf-8")
    t = t.replace(
        """namespace db {
class Database;
}

namespace model {

struct CharacterInstance;
struct CharacterPlayer;

enum class CharacterTemplateType {
""",
        """namespace model {

enum class CharacterTemplateType {
""",
    )
    t = t.replace(
        """bmin::String characterGetSprite(const CharacterTemplate& character);
bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                             int indexOffset);

/** Copy AI/faction fields from template onto a map character instance. */
void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate);

/** Lookup templateName on the database and apply; returns false if missing. */
bool tryApplyCharacterTemplateToInstance(CharacterInstance& character,
                                         const db::Database& database);

/** Copy starting known/ready spell lists from template onto a party member. */
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

} // namespace model
""",
        """bmin::String characterGetSprite(const CharacterTemplate& character);
bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                             int indexOffset);

/** Initialize CharacterStats from a template's stats block. */
void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl);

} // namespace model
""",
    )
    write("model/templates/CharacterTemplate.h", t)

    print("cycle-break patches applied (partial — run full restore flow if needed)")


if __name__ == "__main__":
    main()
