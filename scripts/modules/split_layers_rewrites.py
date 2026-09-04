#!/usr/bin/env python3
"""One-shot: retarget layer .cpp units and LAYER_ID / import sites after the split."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"

STEM_TO_ID = {
    "LayerDropConfirm": "DropConfirm",
    "LayerEquipRunes": "EquipRunes",
    "LayerGiveContext": "GiveContext",
    "LayerInventory": "Inventory",
    "LayerInventoryContext": "InventoryContext",
    "LayerMagic": "Magic",
    "LayerPickUp": "PickUp",
    "LayerPopupText": "PopupText",
    "LayerSpecialEvent": "SpecialEvent",
    "LayerSpellCast": "SpellCast",
    "LayerSpellInfo": "SpellInfo",
    "LayerWorld": "World",
}

CLASS_TO_ID = {
    "LayerDropConfirm": "DropConfirm",
    "LayerEquipRunes": "EquipRunes",
    "LayerGiveContext": "GiveContext",
    "LayerInventory": "Inventory",
    "LayerInventoryContext": "InventoryContext",
    "LayerMagic": "Magic",
    "LayerPickUp": "PickUp",
    "LayerPopupText": "PopupText",
    "LayerSpecialEvent": "SpecialEvent",
    "LayerSpellCast": "SpellCast",
    "LayerSpellInfo": "SpellInfo",
    "LayerWorld": "World",
}

SHOW_ACTION_LEAF = {
    "UiShowLayerInventory.cppm": "carcer.layers.LayerInventory",
    "UiShowLayerInventoryContext.cppm": "carcer.layers.LayerInventoryContext",
    "UiShowLayerMagic.cppm": "carcer.layers.LayerMagic",
    "UiShowLayerSpellCast.cppm": "carcer.layers.LayerSpellCast",
    "UiShowLayerSpellInfo.cppm": "carcer.layers.LayerSpellInfo",
    "UiShowLayerEquipRunes.cppm": "carcer.layers.LayerEquipRunes",
    "UiShowLayerPickUp.cppm": "carcer.layers.LayerPickUp",
    "UiShowLayerPickupContext.cppm": "carcer.layers.LayerPickUpContext",
    "UiShowLayerDropContext.cppm": "carcer.layers.LayerDropConfirm",
    "UiShowLayerGiveContext.cppm": "carcer.layers.LayerGiveContext",
    "UiShowLayerPopupText.cppm": "carcer.layers.LayerPopupText",
    "UiShowLayerSpecialEvent.cppm": "carcer.layers.LayerSpecialEvent",
    "UiCancelEquipRunes.cppm": "carcer.layers.LayerEquipRunes",
}

LEAVES = [
    "carcer.layers.LayerDropConfirm",
    "carcer.layers.LayerEquipRunes",
    "carcer.layers.LayerGiveContext",
    "carcer.layers.LayerInventory",
    "carcer.layers.LayerInventoryContext",
    "carcer.layers.LayerMagic",
    "carcer.layers.LayerPickUp",
    "carcer.layers.LayerPickUpContext",
    "carcer.layers.LayerPopupText",
    "carcer.layers.LayerSpecialEvent",
    "carcer.layers.LayerSpellCast",
    "carcer.layers.LayerSpellInfo",
    "carcer.layers.LayerWorld",
]


def write_text(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", newline="\n")


def rewrite_layer_cpp(path: Path) -> None:
    stem = path.stem
    text = path.read_text(encoding="utf-8")
    text = text.replace("module carcer.layers;", f"module carcer.layers.{stem};", 1)
    enum = STEM_TO_ID.get(stem)
    if enum:
        text = text.replace("Layer(_window, LAYER_ID)", f"Layer(_window, LayerId::{enum})")
        text = text.replace(
            "UiRemoveLayer(bmin::String(LAYER_ID.data(), LAYER_ID.size()))",
            f"UiRemoveLayer(LayerId::{enum})",
        )
        text = text.replace("ObserverRemoveLayer(LAYER_ID)", f"ObserverRemoveLayer(LayerId::{enum})")
        text = text.replace(
            "fromStringView(LAYER_ID)",
            f"fromStringView(layerIdString(LayerId::{enum}))",
        )
        if "LAYER_ID" in text:
            text = text.replace("LAYER_ID", f"layerIdString(LayerId::{enum})")
    write_text(path, text)
    print("retarget", path.relative_to(ROOT).as_posix())


def replace_class_layer_ids(text: str) -> str:
    # Longer class names first so InventoryContext wins over Inventory.
    for cls, enum in sorted(CLASS_TO_ID.items(), key=lambda kv: -len(kv[0])):
        text = text.replace(f"layers::{cls}::LAYER_ID", f"layers::LayerId::{enum}")
        text = text.replace(f"{cls}::LAYER_ID", f"LayerId::{enum}")
    return text


def main() -> None:
    for cpp in sorted((SRC / "layers" / "ui").glob("Layer*.cpp")):
        rewrite_layer_cpp(cpp)

    for path in SRC.rglob("*"):
        if path.suffix not in {".cpp", ".cppm"}:
            continue
        if "lib/sdl2w" in path.as_posix() or "lib\\sdl2w" in str(path):
            continue
        text = path.read_text(encoding="utf-8")
        text = text.replace(
            "UiRemoveLayer(bmin::String(layers::LayerSpellCast::LAYER_ID.data(),\n"
            "                               layers::LayerSpellCast::LAYER_ID.size()))",
            "UiRemoveLayer(layers::LayerId::SpellCast)",
        )
        new = replace_class_layer_ids(text)
        if new != text:
            write_text(path, new)
            print("ids", path.relative_to(ROOT).as_posix())

    for name, leaf in SHOW_ACTION_LEAF.items():
        path = next(SRC.rglob(name))
        text = path.read_text(encoding="utf-8")
        new = text.replace("import carcer.layers;", f"import {leaf};")
        if new == text:
            raise SystemExit(f"no carcer.layers import in {path}")
        write_text(path, new)
        print("action", path.relative_to(ROOT).as_posix(), "->", leaf)

    umbrella = SRC / "modules" / "carcer.cppm"
    u = umbrella.read_text(encoding="utf-8")
    needle = "export import carcer.layers;\n"
    if needle not in u:
        raise SystemExit("umbrella layers import missing")
    extra = "".join(f"export import {leaf};\n" for leaf in LEAVES)
    if "export import carcer.layers.LayerInventory;" not in u:
        u = u.replace(needle, needle + extra, 1)
        write_text(umbrella, u)
        print("umbrella leaves")


if __name__ == "__main__":
    main()
