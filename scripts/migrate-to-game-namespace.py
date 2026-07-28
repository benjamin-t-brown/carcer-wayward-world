#!/usr/bin/env python3
"""One-shot migration: model/ top-level -> game/ namespace."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "src"

MOVED_HEADERS = [
    "MapWalkability.h",
    "MapVision.h",
    "TileFields.h",
    "TileTriggers.h",
    "MapPersistence.h",
]

MODEL_TYPES = [
    "MapInstance",
    "TileInstance",
    "World",
    "Player",
    "CharacterInstance",
    "OpenedDoorRecord",
    "ExploredMapMask",
    "PersistentMapState",
    "PersistentTileFieldRecord",
    "CarcerMapTemplate",
    "DefeatedCharacterRecord",
    "TileMetadata",
    "TilesetTemplate",
    "TileOverrides",
    "TileXY",
    "MapMarkerPlacement",
    "TravelTrigger",
    "TileEventTrigger",
    "ItemInstance",
    "CharacterPlayer",
]

GAME_SYMBOLS = [
    "TILE_FIELD_SPRITE_SHEET",
    "TILE_FIELD_MOVES_PER_COMBAT_ROUND",
    "TILE_FIELD_BLOOD_MOVE_DURATION",
    "TILE_FIELD_FLAME_MOVE_DURATION",
    "TileFieldType",
    "TileField",
    "kPlayerVisionBoxSize",
    "findTileMetadata",
    "resolveTileMetadata",
    "isTileEffectivelyWalkable",
    "isClosedDoorTile",
    "isOpenDoorTile",
    "captureOpenedDoors",
    "applyOpenedDoors",
    "collectTilesAt",
    "resolveTileToRender",
    "isTileCurrentlyVisible",
    "tileAtCurrentLayer",
    "isDestinationWalkable",
    "findClosedDoorAt",
    "isInPlayerVisionRange",
    "isTileEffectivelySeeThrough",
    "doesTileBlockSight",
    "isDestinationSeeThrough",
    "updateMapVisibilityFromPlayer",
    "updateMapVisibilityFromParty",
    "captureExploredMask",
    "applyExploredMask",
    "tileFieldExtraSpriteIndex",
    "tileFieldSpriteName",
    "tileFieldDefaultMoveDuration",
    "ageTileFields",
    "ageMapInstanceTileFields",
    "agePersistentTileFieldRecords",
    "addTileField",
    "addTileFieldAt",
    "findPartyAvatarOnMap",
    "placePartyAvatarAt",
    "queueStepTriggersAt",
    "queueActionTravelAtStanding",
    "formatExamineMessage",
    "flushMapInstance",
    "hydrateMapInstance",
    "flushCurrentMapToPersistence",
    "hydrateCurrentMapFromPersistence",
    "enterMap",
    "markMapCharacterDefeated",
    "advanceWorldMovementTicks",
]


def qualify_model_types_in_game_file(text: str) -> str:
    lines = []
    for line in text.splitlines(keepends=True):
        if line.lstrip().startswith("#include"):
            lines.append(line)
            continue
        for typ in MODEL_TYPES:
            line = re.sub(rf"(?<!model::)\b{typ}\b", f"model::{typ}", line)
        line = line.replace("model::model::", "model::")
        lines.append(line)
    return "".join(lines)


def update_game_file(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    for h in MOVED_HEADERS:
        text = text.replace(f'"model/{h}"', f'"game/{h}"')
    text = re.sub(r"namespace model\b", "namespace game", text)
    if path.suffix == ".cpp" or path.suffix == ".h":
        text = qualify_model_types_in_game_file(text)
    path.write_text(text, encoding="utf-8")


def update_includes_and_symbols(path: Path) -> None:
    if path.match("scripts/*"):
        return
    text = path.read_text(encoding="utf-8")
    original = text
    for h in MOVED_HEADERS:
        text = text.replace(f'"model/{h}"', f'"game/{h}"')
    for sym in GAME_SYMBOLS:
        text = re.sub(rf"\bmodel::{sym}\b", f"game::{sym}", text)
    if text != original:
        path.write_text(text, encoding="utf-8")


def update_makefile(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    replacements = [
        ("model/MapWalkability.cpp", "game/map/MapWalkability.cpp"),
        ("model/MapVision.cpp", "game/map/MapVision.cpp"),
        ("model/TileFields.cpp", "game/map/TileFields.cpp"),
        ("model/MapPersistence.cpp", "game/map/MapPersistence.cpp"),
        ("model/TileTriggers.cpp", "game/map/TileTriggers.cpp"),
    ]
    for old, new in replacements:
        text = text.replace(old, new)
    path.write_text(text, encoding="utf-8")


def main() -> None:
    for path in (ROOT / "game").glob("*"):
        if path.suffix in {".cpp", ".h"}:
            update_game_file(path)

    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix not in {".cpp", ".h", ".hpp", ".mdc"}:
            continue
        if path.parts[-2] == "game" and path.suffix in {".cpp", ".h"}:
            continue
        update_includes_and_symbols(path)

    update_makefile(ROOT / "Makefile")
    print("Migration complete.")


if __name__ == "__main__":
    main()
