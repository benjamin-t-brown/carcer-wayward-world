# CEditor2 database contract audit

Status: Phase 0 audit, based on repository commit `15c5f0b` on 2026-09-09.

This document records the contract CEditor2 must preserve before it introduces a
new schema or migration. It is descriptive of the checked-in JSON, the current
CEditor, and the C++ game loaders. Where those disagree, the disagreement is
called out instead of choosing a new behavior implicitly.

## Sources and precedence

The sources inspected were:

- `ceditor/src/shared/assetRegistry.ts`
- `ceditor/src/client/types/{assets,ability,spell}.ts`
- `ceditor/src/client/utils/{assetLoader,assetNormalizers,jsonUtils,mapIndex}.ts`
- `ceditor/src/client/tile-editor/mapTileItems.ts`
- `ceditor/src/client/contexts/AssetsContext.tsx`
- `ceditor/src/server/index.ts`
- every `src/assets/db/*.json` file
- `src/db/Database.{h,cpp}`, all `src/db/loaders/Load*.cpp`, and the corresponding
  `src/model/templates/*` definitions
- runtime consumers for map travel, character talk, map construction, spells,
  inventory, and special events

For game compatibility, the C++ loaders are authoritative. TypeScript interfaces
describe editor intent but are not runtime validation. The checked-in files are
authoritative for data that the C++ runtime intentionally ignores but the editor
must preserve, such as event-node positions and tileset terrain-paint metadata.

All current JSON database files have an array at the root. All identifiers and
references are case-sensitive strings.

## File inventory and ownership

The proposed classifications below are conservative. “Required” means
`db::Database::load()` opens the file unconditionally. “Optional” means CEditor
currently exposes it but the game does not load it. “Unmanaged” means neither the
current registry nor the game database loads it.

| File | Records | Current CEditor registry id | C++ loader / model | Classification for CEditor2 |
| --- | ---: | --- | --- | --- |
| `status-effects.json` | 1 | `statusEffectTemplates` | `LoadStatusEffectTemplates.cpp` / `StatusEffects.hpp` | required, managed |
| `abilities.json` | 8 | `abilityTemplates` | `LoadAbilityTemplates.cpp`, `LoadAbilityJson.cpp` / `Abilities.hpp` | required, managed |
| `items.json` | 18 | `itemTemplates` | `LoadItemTemplates.cpp` / `Items.h` | required, managed |
| `spells.json` | 3 | `spellTemplates` | `LoadSpellTemplates.cpp` / `Spells.hpp` | required, managed |
| `characters.json` | 9 | `characterTemplates` | `LoadCharacterTemplates.cpp` / `CharacterTemplate.h` | required, managed |
| `maps.json` | 25 | `maps` | `LoadMapTemplates.cpp` / `Maps.h` | required, managed |
| `map-grids.json` | 2 | `mapGrids` | `LoadMapGridTemplates.cpp` / `MapGrids.hpp` | required, managed |
| `tilesets.json` | 4 | `tilesetTemplates` | `LoadTilesetTemplates.cpp` / `Tileset.hpp` | required, managed |
| `special-events.json` | 33 | `specialEvents` | `LoadSpecialEvents.cpp` / `SpecialEvents.hpp` | required, managed |
| `feats.json` | absent | `featTemplates` | none; editor types explicitly say editor-only | optional/prospective; decision required |
| `tiles.json` | 1 | none | none found | unmanaged/orphaned; decision required |

The registry contains ten entries because it includes nonexistent `feats.json`
and omits existing `tiles.json`. A GET for a registered missing file returns
`[]`; the first POST creates that file. Thus the current editor presents feats
as a valid empty collection despite there being no game consumer.

`tiles.json` contains a legacy-looking tile record:

```text
{ type, spritesheet, spriteOffset, stepSound }
```

No CEditor or C++ reference to `tiles.json` was found. Tile behavior used by the
game is stored under `tilesets.json`. Do not include `tiles.json` in Save All
until its ownership is decided.

The `.txt` and `.se` files below `src/assets/db` are also outside the registered
JSON contract. The media definitions named `src/assets/assets.*.txt` are read by
CEditor through `/api/sdl2w-assets`, but are a read-only sprite/animation/sound
catalog rather than database files. CEditor2 Save All should not rewrite any of
these files unless their scope is explicitly expanded.

## JSON record shapes

Notation: unmarked fields are required by the C++ loader; `?` means optional or
defaulted by it. Editor-only fields must still survive a round trip.

### Abilities

```text
Ability = {
  name: string, label: string, description: string, icon: SpriteName,
  type: AbilityType,
  targetSelect: {
    targetType: TargetSelectType,
    allegianceSelectType: TargetAllegianceSelectType,
    numTargetableUnits: integer,
    zoneSize: { x: integer, y: integer },
    range: integer
  },
  apCost: integer,
  costType: AbilityCostType,
  costValue: integer,
  depiction: {
    dmgAnim: AnimationName,
    projectileType: ProjectileType,
    projectilePath: ProjectilePath,
    startSound: SoundName,
    dmgSound: SoundName
  },
  attacks?: Attack[], statuses?: AppliedStatus[],
  restores?: Restore[], damages?: Damage[]
}

Attack = {
  attackClass: AttackClass,
  damageType?: DamageType,
  dmg?: AttackDamage,
  save?: Save
}
AttackDamage = {
  dmgDice: DiceName[], dmgBonus: integer, dmgStat: StatName,
  dmgStatMult: number, attackBonus: integer
}
Save = {
  saveStat: StatName, saveBase: integer,
  saveAgainst: StatName, saveAgainstBase: integer
}
AppliedStatus = {
  statusEffect: StatusEffectName,
  save?: Save, baseDuration?: integer, durationBonus?: integer
}
Restore = {
  restoreWhich: CurrentStatName, restoreDice: DiceName[],
  restoreBonus: integer, restoreStat: StatName, restoreStatMult: integer
}
Damage = {
  damageType: DamageType, dmgDice: DiceName[], dmgBonus: integer,
  dmgStat: StatName, dmgStatMult: number
}
```

The C++ depiction loader also accepts legacy `projectileAnim` in place of
`projectileType`. New output should use `projectileType`. The TypeScript
`AbilityAttack` requires `damageType`, while the C++ loader defaults it when
missing. Conversely, C++ requires `restoreStatMult` to be an integer even though
the TypeScript type only says `number`.

Authoritative consumers are `LoadAbilityTemplates.cpp`, `LoadAbilityJson.cpp`,
`Abilities.hpp`, and the combat rules. Duplicate `name` values are rejected.

### Status effects

```text
StatusEffect = {
  name: string,
  description: string,
  baseDuration: integer,
  durationScale?: { durationStat: StatName, durationStatMult: integer },
  applyBonuses?: { STR?: integer, MND?: integer, CON?: integer,
                    AGI?: integer, LCK?: integer },
  applyCurrentStatChange?: { HP?: integer, AP?: integer,
                              MANA?: integer, AC?: integer },
  applyResistances?: { attackType: DamageType, mod: integer }[],
  actions?: {
    statusActionTargetType: StatusActionTargetType,
    abilityName: AbilityName,
    events: { type: StatusEventType,
              condition: StatusEffectCondition }[]
  }[]
}
```

Legacy top-level `duration`, `events`, and `targetInfo` are explicitly rejected,
not normalized by the game. Duplicate names are rejected.

Authoritative consumers are `LoadStatusEffectTemplates.cpp` and
`StatusEffects.hpp`.

### Items

```text
Item = {
  itemType: ItemType,
  name: string, label: string, icon: SpriteName, description: string,
  weight: integer, value: integer,
  stackable?: boolean,                 // runtime default false
  indestructable?: boolean,            // spelling is part of the contract
  itemUsability?: ItemUsability,       // runtime default NOT_USABLE
  statusEffects?: (StatusEffectName | { name: StatusEffectName })[],
  useAbility?: {
    abilityName: AbilityName,
    dmgOverrides?: AttackDamage[],
    restoreOverrides?: Restore[]
  },
  useSpecialEvent?: SpecialEventId,
  weapon?: {
    abilityName: AbilityName,
    dmgOverrides?: AttackDamage[]
  },
  runeType?: RuneType                  // required iff itemType == RUNE
}
```

The loader accepts legacy weapon `{ attackIndex?, dmg }`, converting it to the
indexed override array. The editor removes legacy `itemUsabilityArgs`, migrates
legacy weapon damage, removes weapon data from non-weapon items, and removes
`runeType` from non-runes. `useAbility` and `useSpecialEvent` are alternatives in
the current form, although the loader can store both. Duplicate names and invalid
item types are rejected.

Authoritative consumers are `LoadItemTemplates.cpp`, `Items.h`, inventory rules,
and item-use actions.

### Spells

```text
Spell = {
  name: string, label: string, description: string, icon: SpriteName,
  abilityName: AbilityName,
  requiredRunes: { type: RuneType, count: positive integer }[]
}
```

Rune types may not repeat within one spell. All fields are required, duplicate
spell names are rejected, and `Database::validateCombatReferences()` requires
`abilityName` to exist.

Authoritative consumers are `LoadSpellTemplates.cpp`, `Spells.hpp`, and
`SpellRules.cpp`.

### Characters

```text
Character = {
  type: CharacterTemplateType,
  name: string, label: string,
  spritesheet: PictureOrSpriteBaseName,
  spriteOffset: integer | integer-string,
  talk?: { talkName?: SpecialEventId, portraitName?: SpriteName },
  behavior?: { behaviorName?: CharacterBehaviorName },
  stats?: {
    generic?: { str?, mnd?, con?, agi?, lck?: integer },
    trainable?: {
      weapon?: { edged?, pole?, blunt?, range?, unarmed?: integer },
      magic?: { mana?, abilityPower?, attunement?, faith?, lore?: integer },
      body?: { resistPhysical?, resistMagical?, healingEffectiveness?,
               dr?, armorTraining?: integer }
    },
    skills?: { trickery?, stealth?, social?, magicItemUse?, cooking?,
               acrobatics?, survival?, focus?, conditioning?: integer }
  },
  combat?: { hp?: integer, mp?: integer, dropTable?: string,
             stats?: LegacyGenericStats },
  combatBehavior?: { town?: CombatBehaviorName, combat?: CombatBehaviorName },
  sound?: {
    deathSoundName?: SoundName, weaponSoundName?: SoundName,
    deathSound?: SoundName, weaponSound?: SoundName
  },
  statuses?: { status: StatusEffectName }[],
  vision?: { radius?: integer },
  startingKnownSpells?: SpellName[],
  startingReadySpells?: SpellName[]
}
```

Only `type`, `name`, `label`, `spritesheet`, and `spriteOffset` are required by
the loader. `combat.stats` is a legacy overlay applied after `stats.generic`.
The old sound keys remain accepted, with the `*SoundName` keys taking precedence.
Although the C++ loader and model support `startingKnownSpells` and
`startingReadySpells`, the current TypeScript `CharacterTemplate` omits them;
CEditor2 must add them to its lossless model. Duplicate names are rejected.

Authoritative consumers are `LoadCharacterTemplates.cpp`,
`CharacterTemplate.{h,cpp}`, character construction, and world talk.

### Maps

```text
Map = {
  name: string,
  label?: string,
  type?: TOWN | OUTDOOR,
  width?: integer, height?: integer,
  spriteWidth?: integer, spriteHeight?: integer,
  tilesets?: string[],                 // index 0 conventionally ""
  layers?: integer[],
  tiles?: { [layerNumberAsString]: [tilesetIndex, tileId, ...] },
  characters?: { l: integer, i: integer, name: CharacterName }[],
  items?: { l: integer, i: integer, name: ItemName, quantity?: integer }[],
  markers?: { l: integer, i: integer, name: string }[],
  eventTriggers?: {
    l: integer, i: integer, eventId: SpecialEventId,
    requiresNonCombat?: boolean,       // runtime default true
    requiresLook?: boolean,            // runtime default false
    overlayVisibility?: OverlayVisibility
  }[],
  travelTriggers?: {
    l: integer, i: integer,
    destinationMapName: MapName | MapGridName,
    destinationMarkerName?: marker name scoped to destination map,
    destinationX?: integer, destinationY?: integer,
    destinationLayer?: integer,
    requiresAction?: boolean,
    overlayVisibility?: OverlayVisibility
  }[],
  tileOverrides?: {
    l: integer, i: integer,
    overrides: {
      isWalkableOverride?: boolean,
      isSeeThroughOverride?: boolean,
      isContainerOverride?: boolean,
      lightSourceOverride?: { angle?: integer, intensity?: integer,
                              radius?: integer }
    }
  }[],
  lightSources?: { l: integer, i: integer,
                    angle?: integer, intensity?: integer, radius?: integer }[]
}
```

`i` is row-major (`y * width + x`) and `l` is the logical layer. Each dense
graphics array must contain `width * height * 2` integers. Tileset indices address
the map-local `tilesets` dictionary. The editor adds `""` at index zero, creates
missing arrays, and materializes legacy `{ levels: ... }` maps into the flat
shape. It normalizes item quantities to integers of at least one. Hidden overlay
visibility is normally omitted on serialization and defaults to `HIDDEN`.

Critical mismatch: current maps use layers `-1`, `0`, and `1`, and five maps have
a negative layer. `LoadMapTemplates.cpp` only copies a `tiles` entry when the
parsed layer is `>= 0`; `CarcerMapTemplate.tiles` is indexed as an unsigned array.
The game therefore ignores negative-layer graphics even though CEditor models and
edits them. This needs a deliberate runtime or data migration decision before map
work begins.

Only `name` is strictly required by the C++ parser, but CEditor2 should validate
all structural invariants above. Duplicate map names are rejected.

Authoritative consumers are `LoadMapTemplates.cpp`, `Maps.{h,cpp}`,
`MapInstance.cpp`, map persistence, travel, and rendering.

### Map grids

```text
MapGrid = {
  name: string,
  label?: string,
  gridWidth?: positive integer,
  gridHeight?: positive integer,
  mapWidth?: positive integer,
  mapHeight?: positive integer,
  cells?: string[][]                  // row-major [y][x], "" = empty
}
```

The loader and editor clamp all dimensions to at least one, resize `cells` to
exactly `gridHeight` by `gridWidth`, preserve the overlapping region, and fill
missing/non-string cells with `""`. Duplicate names are rejected.

Authoritative consumers are `LoadMapGridTemplates.cpp`, `MapGrids.hpp`, and
`ActiveMapOrchestrator.cpp`.

### Tilesets

```text
Tileset = {
  name: string,
  spriteBase?: string,
  imageWidth?: integer,               // editor-only
  imageHeight?: integer,              // editor-only
  tileWidth?: integer,
  tileHeight?: integer,
  terrain?: {                         // editor-only terrain paint metadata
    primaryTerrain: TerrainTag,
    secondaryTerrain: TerrainTag,
    mode: integer,
    startTileId: integer
  },
  tiles?: {
    id?: integer,
    description?: string,
    stepSound?: integer | integer-string,
    isWalkable?: boolean,
    isSeeThrough?: boolean,
    isDoor?: boolean,
    isContainer?: boolean,
    tileTerrainBorderMeta?: { ne: TerrainTag, nw: TerrainTag,
                              se: TerrainTag, sw: TerrainTag } // editor-only
  }[]
}
```

The C++ loader requires only `name`. It reads `spriteBase`, tile dimensions, and
the listed gameplay tile fields. It ignores `imageWidth`, `imageHeight`,
`terrain`, `tileTerrainBorderMeta`, and unknown properties. These ignored fields
are nevertheless required by editor behavior and must be preserved. Step sounds
map `0..3` to floor/grass/dirt/gravel; invalid types and values fall back to
floor. Duplicate names are rejected.

Authoritative game consumers are `LoadTilesetTemplates.cpp`, `Tileset.hpp`, map
walkability/vision, and map rendering. The checked-in JSON is authoritative for
the editor-only fields.

### Special events

```text
SpecialEvent = {
  id: string, title: string,
  eventType: MODAL | TALK,
  icon: SpriteName,
  vars?: { id?: string, key?: string, value?: string,
            importFrom?: SpecialEventId }[],
  children?: EventNode[]
}

EventNode common editor fields = {
  id: string, eventChildType: string,
  x?: number, y?: number, h?: number   // editor layout only
}
EXEC    += { p?: string, execStr?: string, next?: NodeId,
             autoAdvance?: boolean, audioInfo?: AudioInfo }
CHOICE  += { text?: string, choices?: Choice[], audioInfo?: AudioInfo }
Choice   = { text?: string, switchText?: { conditionStr?: string,
                                           text?: string }[],
             prefixText?: string, conditionStr?: string, evalStr?: string,
             next?: NodeId }
SWITCH  += { defaultNext?: NodeId,
             cases?: { conditionStr?: string, next?: NodeId }[] }
END     += { next?: NodeId }
COMMENT += { comment?: string }        // editor-only
AudioInfo = { audioName?: SoundName, volume?: integer, offset?: integer }
```

The C++ loader requires the four event header fields. Each child requires
`eventChildType` and `id`; most child-specific fields otherwise default to empty
values. It accepts only event types `MODAL` and `TALK`. The TypeScript `GameEvent`
also mentions `TRAVEL`, but the game rejects it.

`COMMENT` nodes are intentionally discarded by the game. `KEYWORD` exists in the
TypeScript enums/types but is not supported by the current C++ child parser.
More broadly, the loader catches every exception while parsing an individual
child and silently skips that child. CEditor2 validation should report these
cases before save rather than relying on silent runtime loss. The checked-in file
currently contains 387 EXEC, 72 CHOICE, 42 END, 38 SWITCH, and 15 COMMENT nodes.

`p` is stored as a string and split into paragraphs by the game. `next`,
`defaultNext`, choice `next`, and case `next` refer to node ids inside the same
event. `conditionStr`, `evalStr`, and `execStr` are DSL source strings. They may
contain implicit item, character, shop, quest, map, or other game identifiers;
they cannot be treated as ordinary text during reference renames.

Authoritative consumers are `LoadSpecialEvents.cpp`, `SpecialEvents.hpp`, and
the `src/in3` evaluators/runners. The checked-in JSON is authoritative for editor
layout and comments.

### Feats (prospective only)

The editor-only draft type is:

```text
Feat = {
  id: string, label: string,
  description?: string,
  implementation?: DATA | CUSTOM,
  excludesGroup?: string,
  requiresFeats?: FeatId[],
  effects?: { type: string, when?: string, target?: string,
              op?: string, value?: number }[]
}
```

There is no checked-in file, loader, runtime model, or authoritative schema.
Treating this as a Save All participant would create `feats.json`; that is a
product/schema decision, not a compatibility requirement.

## Cross-asset reference matrix

| Source field | Target namespace | Runtime behavior / validation |
| --- | --- | --- |
| `abilities[].statuses[].statusEffect` | `status-effects[].name` | hard-validated by `Database::validateCombatReferences()` |
| `status-effects[].actions[].abilityName` | `abilities[].name` | hard-validated by `Database::validateCombatReferences()` |
| `items[].statusEffects[]` | `status-effects[].name` | hard-validated by `Database::validateCombatReferences()` |
| `items[].weapon.abilityName` | `abilities[].name` | consumed by item/combat rules; not validated during database load |
| `items[].useAbility.abilityName` | `abilities[].name` | consumed on use; not validated during database load |
| `items[].useSpecialEvent` | `special-events[].id` | consumed on use; not validated during database load |
| `spells[].abilityName` | `abilities[].name` | hard-validated by `Database::validateCombatReferences()` |
| `characters[].talk.talkName` | `special-events[].id` | checked at interaction time; missing event logs/falls back |
| `characters[].statuses[].status` | `status-effects[].name` | loaded as a reference; no database-load validation found |
| `characters[].startingKnownSpells[]` | `spells[].name` | copied to player; no database-load validation found |
| `characters[].startingReadySpells[]` | known spell subset / `spells[].name` | only entries also in known list are retained; existence not validated there |
| `maps[].tilesets[n]` | `tilesets[].name` | map graphics resolve indirectly through numeric `tilesetIndex`; not load-validated |
| `maps[].characters[].name` | `characters[].name` | becomes `CharacterInstance.templateName`; lookups may fail/fall back later |
| `maps[].items[].name` | `items[].name` | becomes `ItemInstance.itemTemplateName`; not load-validated |
| `maps[].eventTriggers[].eventId` | `special-events[].id` | invoked later; not load-validated |
| `maps[].travelTriggers[].destinationMapName` | `maps[].name` or `map-grids[].name` | resolved at travel time; failure is logged |
| `maps[].travelTriggers[].destinationMarkerName` | marker name within destination map | resolved at travel time, then falls back to destination coordinates |
| `map-grids[].cells[][]` | `maps[].name` or empty string | used by active-map orchestration; not load-validated |
| `special-events[].vars[].importFrom` | `special-events[].id` | resolved while initializing runner variables; current editor supports rename |
| event-node `next` fields | node `id` in same event | runtime graph edge; should be validated per event |
| event DSL strings | several domain namespaces | implicit/parsed references; requires DSL-aware analysis |
| `feats[].requiresFeats[]` | `feats[].id` | prospective only; no runtime |

Media references are not JSON-to-JSON references, but must be validated against
the parsed SDL2W catalog: item/ability/spell/event icons and portraits refer to
sprites; ability depiction refers to animations/sounds; character sound fields
and event `audioInfo.audioName` refer to sounds; character/tileset sprite bases
refer to picture/sprite definitions.

The current event rename helper updates map event triggers, character talk ids,
item `useSpecialEvent`, and event `importFrom`. Ability deletion helpers update
item weapon/use configs and status-effect actions. These operations currently
save only the explicitly affected files and are not transactional.

## Current load normalization

The current server returns parsed JSON without validation. Missing registered
files become `[]`; malformed JSON produces HTTP 500. The browser loads every
registered collection in parallel, parses the SDL2W media catalog, and then runs
these normalizers:

| Collection | Current browser normalization |
| --- | --- |
| abilities | converts projectile aliases, maps invalid/legacy target types to `TARGET_UNIT`, and clears animation/sound names absent from the media catalog |
| spells | reconstructs each record from known fields; drops invalid/duplicate rune requirements and floors positive rune counts |
| items | runs after abilities; removes legacy/inapplicable fields and expands weapon/use damage and restore overrides from referenced base abilities |
| maps | migrates legacy `levels` to flat arrays; ensures layers, tile dictionaries, placement arrays, and dense graphics; clamps item quantities |
| map grids | clamps dimensions and resizes cells |
| status effects, characters, feats, special events, tilesets | raw pass-through |

These transformations are not all lossless. In particular, ability media names
can be cleared merely because the editor's media catalog did not load them,
spell unknown fields are dropped, item legacy forms are rewritten, and maps are
migrated eagerly. CEditor2 should separate three concepts:

1. parse without mutation and preserve the source document;
2. derive a safe editor view/defaults;
3. run an explicit, tested migration when the stored representation must change.

A no-edit Save All must not introduce semantic changes or discard unknown and
editor-only fields.

## Serialization expectations

Observed current behavior:

- Each page calls one `POST /api/assets/:type`; “Save All” means all records in
  that page, not all database files.
- The server replaces the target directly with `JSON.stringify(body, null, 2)`.
  There is no staging, revision check, rollback, server-side schema validation,
  or final newline.
- Before save, pages recursively trim every string, including prose, DSL source,
  ids, and values.
- Abilities, status effects, spells, items, characters, tilesets, map grids, feats,
  and special events are sorted by their name/id (with a label tiebreaker for
  items, characters, and feats). Maps intentionally preserve their current order.
- `undefined` properties disappear through JSON serialization; object property
  order otherwise follows JavaScript insertion order.

CEditor2 compatibility requirements:

- serialize every managed file as a root array with two-space indentation;
- choose and test deterministic record ordering per collection (preserve map
  order unless a migration explicitly changes it);
- retain unknown, editor-only, and currently unsupported fields during a no-op
  round trip;
- do not globally trim narrative text or DSL strings without a field-specific
  rule; trim identifiers where the contract requires canonical ids;
- validate the C++-required types, enum values, unique ids, dense map sizes,
  reference matrix, and special-event graph before committing;
- stage and commit the complete managed snapshot with conflict detection and
  rollback, rather than issuing unrelated per-file writes;
- make migrations explicit and versioned/tested; loading alone must not rewrite
  data;
- decide whether to add a final newline. Existing managed files written by the
  server end in `]` with no newline, while `tiles.json` ends with a newline. Either
  choice is semantically safe, but deterministic output is required.

## Findings in the checked-in corpus

The audit parsed all ten existing JSON files and found:

- all have array roots and all identity-bearing collections have unique ids;
- all current map graphics arrays have exactly `width * height * 2` entries;
- all current map-grid cell matrices match their declared dimensions;
- event `importFrom`, spell-to-ability, map-to-tileset, map event, map item,
  character talk, grid map, and map travel references resolve exactly;
- `PotionHealing.useAbility.abilityName` is `HEAL_SELF`, but no ability with that
  name exists (there is `SPELL_HEAL_SELF_MINOR`); this is not caught by the C++
  database validator;
- maps reference character `exampleTownsperson_2`, but the character collection
  contains `exampleTownsperson`; this is also not load-validated;
- five maps declare a negative layer that the C++ loader ignores, as described
  above;
- no current ability applies a status, and no current item/character contains a
  status reference, so those paths are supported by code but not exercised by
  the current corpus;
- `tiles.json` is the only existing JSON file with no discovered consumer.

These findings should initially be validation diagnostics, not automatic fixes.

## Decisions required before Phase 1/2

1. Is `feats.json` a real optional managed collection to be created, or should
   the unfinished feat editor be excluded until the game has a schema/consumer?
2. Should `tiles.json` be deleted/migrated, retained as unmanaged legacy data, or
   brought under a new contract? It must not be silently folded into tilesets.
3. Are negative map layers intended game data? If yes, the C++ representation and
   loader need correction; if no, the five maps need an explicit migration.
4. Should `HEAL_SELF` be changed to `SPELL_HEAL_SELF_MINOR`, or is an ability
   missing? Should `exampleTownsperson_2` be created or renamed in the map data?
5. Should editor-only special-event node types be allowed in saved files with
   warnings, or should unsupported runtime nodes block save? COMMENT is known to
   be safe; KEYWORD is not implemented by the C++ parser.
6. Which DSL identifiers should participate in rename/reference analysis? This
   requires parsing function arguments, not global string replacement.
7. Should the character editor expose the runtime-supported starting spell
   fields and legacy-compatible fields, or only preserve them invisibly?
8. Should deterministic output preserve the current no-final-newline convention?

