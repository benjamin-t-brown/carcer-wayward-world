# Header Restoration Manifest

Status: Phase 0 baseline recorded; production migration not started.

This is the parity ledger for `HEADER_ARCHITECTURE_RESTORATION_PLAN.md`. A row may move from `pending` to `ported` only when its declaration and current behavior have been placed in the target file; it moves to `verified` only after the owning phase gate passes. Class/struct rows account for their public members as one indivisible API surface.

## Fixed references

| Role | Commit | Meaning |
|---|---|---|
| Header layout reference | `fe50a90` | Last pre-module Carcer layout; use for file boundaries, not automatically for behavior |
| Current behavior reference | `d85a0e1` | Completed module experiment including native Windows/MSYS2 fixes |
| Plan checkpoint | `ecf2749` | Plan as committed on preserved `experiment/cpp-modules` |
| Restoration plan on this branch | `36883ff` | Cherry-pick of the plan onto `refactor/header-architecture` |

Dependency revisions approved for the restoration are SDL2W `e5415231257b4a25f25ad3af5cb4e01c125ada15` and BMIN `e60f65b1d3a36def8221bd93c5702d323c714cea`, both from `experiment/cpp-modules`. Carcer will consume their header API; their module API remains an opt-in upstream compatibility concern.

## Baseline host and tools

Recorded 2026-09-07 on macOS 26.6.2 (Darwin 25.6.0), x86_64, 10 logical cores, 64 GB RAM.

| Tool | Version / selection |
|---|---|
| Legacy Make `CXX` | `c++` -> Apple Clang 21.0.0 |
| Module dependency builder observed | GCC 15 (`g++-15`) |
| CMake | 4.4.3 |
| Ninja | 1.13.2 |
| Node | 24.13.1 |
| Python | 3.14.6 |

## Unmodified header baseline

The exact legacy command `make -C src -j8` failed immediately because its mutable, ignored `src/sdl2w` checkout was absent and sandboxed network access prevented the attempted clone. For a meaningful compatibility baseline, it was rerun with only `SDL2W_DIR_NAME=../.deps/sdl2w` and `BMIN_REPO=<repo>/.deps/bmin`, pointing at the approved revisions above. No tracked production file was changed.

| Check | Result |
|---|---|
| Clean eight-job legacy build | **FAIL**, 34.06 s elapsed before failure (93.94 s user, 17.01 s sys). Dependency `all` also rebuilt module artifacts even though Carcer was a header consumer, so this is diagnostic rather than an accepted compile-time baseline. |
| `TestJson.sh` | **FAIL** during shared production-object compilation |
| `TestCharacterEquip.sh` | **FAIL** during shared production-object compilation |
| `TestConfirmModal.sh --build-only` | **FAIL** during shared production-object compilation |

The two primary compatibility failures are: (1) the old Carcer include topology instantiates `bmin::DynArray<sdl2w::Sprite>` through `sdl2w::Animation` while `Sprite` is incomplete; (2) `runner/EventRunnerHelpers.cpp` compares `bmin::Map::Iterator` with `ConstIterator`, which the current BMIN header API rejects. The representative runners compile all legacy production objects before their requested test, so all encounter these unrelated failures. Phase 1/3 must fix them in the owning architecture, not by modifying this baseline.

## Phase 1 local qualification

The conventional header build and pinned dependency staging are implemented in
the current worktree. The following results are local and must be committed only
after the Phase 1 cross-platform disposition is agreed:

| Check | Result |
|---|---|
| Pinned checkout validation | PASS; missing and wrong-revision fixtures both fail with actionable diagnostics and exit status 1 |
| Transitional Make application build | PASS with the pinned header bundle |
| GCC 15 debug / release CMake builds | PASS / PASS |
| Clang 22 debug / release CMake builds | PASS / PASS |
| Immediate GCC debug rebuild | PASS; revision validation only, zero C++ compilation |
| Representative legacy wrappers | `TestJson` PASS, `TestCharacterEquip` PASS, `TestConfirmModal --build-only` PASS |
| Module machinery in Carcer compile commands | PASS; no `.cppm`, `-fmodules-ts`, prebuilt-module, or scan command |
| UCRT64 | Not runnable on this macOS host; commands documented in `DEVELOPMENT.md` |
| Emscripten | Not runnable because no activated/installed EMSDK is present; commands documented in `DEVELOPMENT.md` |

Compatibility changes required to reach the local passes were limited to
canonical BMIN include spelling, the current const-map iterator API, the current
`Store::hasSprite` API, the missing standard `<cmath>` include, and a centralized
`Sdl2wAnimation.h` header-order adapter for the upstream incomplete `Sprite`
declaration.

## Inventory counts

- Header reference production inventory: 248 headers and 136 sources (384 paths total).
- Module behavior reference: 20 module interfaces and 670 exported top-level types/functions (class members are covered by their type row).
- Test inventory at the header reference: 81 C++ test sources and 79 shell wrappers.
- Known intentionally disabled tests: none identified. Known stale tests: not yet classified; the three representative wrappers are blocked before test compilation by production incompatibilities listed above.

## Semantic commit ledger

| Commit | Behavior to preserve | Owning phase | Status |
|---|---|---|---|
| `e0c0b83` | Pinned dependency bootstrap and reproducible validation | 1 | ported; local checks pass |
| `d60964c` | Notification expiry removed from action ownership | 4 | pending |
| `58b4cc9` | Active-map dependencies made explicit | 4 | pending |
| `9e9617e` | Map rules removed from state | 4 | pending |
| `eebd21f` | Combat sequencing moved to actions/orchestration | 4/5 | pending |
| `e25ff69` | Rules/state/action ownership split | 4/5 | pending |
| `ba41411` | Narrow action API and action-event behavior | 5 | pending |
| `5030c80` | Layers above screens; UI does not own layers | 7 | pending |
| `a34939a` | Narrow application composition root | 7/8 | pending |
| `aae59e4` | Boundary checks and architecture documentation | 3/8 | pending |
| `d85a0e1` | Native Windows/MSYS2 and clangd fixes | 1/8 | Windows shim ported; UCRT64 verification pending |

## Exported API migration ledger

Rows are generated from column-zero exported declarations before each interface's private fragment. Nested public member functions are governed by the owning class/struct row. Aggregator interfaces with no declarations remain listed explicitly.

### `src/actions/_actions.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `Command` | class | `src/actions/actions.cpp` | `src/actions/Command.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `adjustEquippedRune` | function | `src/actions/actions.cpp` | `src/actions/adjustEquippedRune.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `cancelEquipRunes` | function | `src/actions/actions.cpp` | `src/actions/cancelEquipRunes.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `commitEquipRunes` | function | `src/actions/actions.cpp` | `src/actions/commitEquipRunes.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `continueSpecialEvent` | function | `src/actions/actions.cpp` | `src/actions/continueSpecialEvent.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `doCombatAction` | function | `src/actions/actions.cpp` | `src/actions/doCombatAction.hpp (proposed; verify during owning phase)` | pending | TestCombatActions |
| `doCPUCombatTurn` | function | `src/actions/actions.cpp` | `src/actions/doCPUCombatTurn.hpp (proposed; verify during owning phase)` | pending | TestEnemyBehavior |
| `dropInventoryItem` | function | `src/actions/actions.cpp` | `src/actions/dropInventoryItem.hpp (proposed; verify during owning phase)` | pending | TestDropInventoryItem |
| `endCombat` | function | `src/actions/actions.cpp` | `src/actions/endCombat.hpp (proposed; verify during owning phase)` | pending | TestCombatActions |
| `examineAt` | function | `src/actions/actions.cpp` | `src/actions/examineAt.hpp (proposed; verify during owning phase)` | pending | TestWorldExamineAt |
| `giveInventoryItem` | function | `src/actions/actions.cpp` | `src/actions/giveInventoryItem.hpp (proposed; verify during owning phase)` | pending | TestCharacterGive |
| `goNextCombatTurn` | function | `src/actions/actions.cpp` | `src/actions/goNextCombatTurn.hpp (proposed; verify during owning phase)` | pending | TestCombatActions |
| `interactAt` | function | `src/actions/actions.cpp` | `src/actions/interactAt.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `loadActiveMap` | function | `src/actions/actions.cpp` | `src/actions/loadActiveMap.hpp (proposed; verify during owning phase)` | pending | TestMapPersistence, TestWorldSpawnPlayerAtMarker, TestWorldTravel |
| `modifyAP` | function | `src/actions/actions.cpp` | `src/actions/modifyAP.hpp (proposed; verify during owning phase)` | pending | TestCombatActions |
| `modifyHP` | function | `src/actions/actions.cpp` | `src/actions/modifyHP.hpp (proposed; verify during owning phase)` | pending | TestCombatActions |
| `modifyPartyMemberHp` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model/action suite |
| `moveActionAim` | function | `src/actions/actions.cpp` | `src/actions/moveActionAim.hpp (proposed; verify during owning phase)` | pending | TestWorldActionAim |
| `movePlayer` | function | `src/actions/actions.cpp` | `src/actions/movePlayer.hpp (proposed; verify during owning phase)` | pending | TestEnemyBehavior, TestTileTriggers, TestWorldMovePlayer |
| `pickUpItem` | function | `src/actions/actions.cpp` | `src/actions/pickUpItem.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `playSound` | function | `src/actions/actions.cpp` | `src/actions/playSound.hpp (proposed; verify during owning phase)` | pending | TestCombatActions |
| `pushFloatingNotification` | function | `src/actions/actions.cpp` | `src/actions/pushFloatingNotification.hpp (proposed; verify during owning phase)` | pending | TestFloatingNotificationSection |
| `removeFloatingNotification` | function | `src/actions/actions.cpp` | `src/actions/removeFloatingNotification.hpp (proposed; verify during owning phase)` | pending | TestStateManagerActions |
| `removeLayer` | function | `src/actions/actions.cpp` | `src/actions/removeLayer.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `reorderInventoryItem` | function | `src/actions/actions.cpp` | `src/actions/reorderInventoryItem.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `selectSpecialEventChoice` | function | `src/actions/actions.cpp` | `src/actions/selectSpecialEventChoice.hpp (proposed; verify during owning phase)` | pending | TestStateManagerActions |
| `selectSpellCast` | function | `src/actions/actions.cpp` | `src/actions/selectSpellCast.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `setActionAim` | function | `src/actions/actions.cpp` | `src/actions/setActionAim.hpp (proposed; verify during owning phase)` | pending | TestWorldActionAim |
| `setActionMode` | function | `src/actions/actions.cpp` | `src/actions/setActionMode.hpp (proposed; verify during owning phase)` | pending | TestWorldActionAim |
| `setActiveCombatCharacter` | function | `src/actions/actions.cpp` | `src/actions/setActiveCombatCharacter.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `setCurrentPartyMember` | function | `src/actions/actions.cpp` | `src/actions/setCurrentPartyMember.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `setCurrentPartyMemberInventory` | function | `src/actions/actions.cpp` | `src/actions/setCurrentPartyMemberInventory.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `setCurrentPartyMemberMagic` | function | `src/actions/actions.cpp` | `src/actions/setCurrentPartyMemberMagic.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `setSelectedPartyMemberId` | function | `src/actions/actions.cpp` | `src/actions/setSelectedPartyMemberId.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `setSpellReady` | function | `src/actions/actions.cpp` | `src/actions/setSpellReady.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerDropContext` | function | `src/actions/actions.cpp` | `src/actions/showLayerDropContext.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerEquipRunes` | function | `src/actions/actions.cpp` | `src/actions/showLayerEquipRunes.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerGiveContext` | function | `src/actions/actions.cpp` | `src/actions/showLayerGiveContext.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerInventory` | function | `src/actions/actions.cpp` | `src/actions/showLayerInventory.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerInventoryContext` | function | `src/actions/actions.cpp` | `src/actions/showLayerInventoryContext.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerMagic` | function | `src/actions/actions.cpp` | `src/actions/showLayerMagic.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerPickUp` | function | `src/actions/actions.cpp` | `src/actions/showLayerPickUp.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerPickupContext` | function | `src/actions/actions.cpp` | `src/actions/showLayerPickupContext.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerPopupText` | function | `src/actions/actions.cpp` | `src/actions/showLayerPopupText.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerSpecialEvent` | function | `src/actions/actions.cpp` | `src/actions/showLayerSpecialEvent.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerSpellCast` | function | `src/actions/actions.cpp` | `src/actions/showLayerSpellCast.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `showLayerSpellInfo` | function | `src/actions/actions.cpp` | `src/actions/showLayerSpellInfo.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `spawnPlayer` | function | `src/actions/actions.cpp` | `src/actions/spawnPlayer.hpp (proposed; verify during owning phase)` | pending | TestCombat, TestLayerWorld |
| `spawnPlayerAtMarker` | function | `src/actions/actions.cpp` | `src/actions/spawnPlayerAtMarker.hpp (proposed; verify during owning phase)` | pending | TestWorldSpawnPlayerAtMarker, TestCombat, TestLayerWorld |
| `spawnPlayerAtXY` | function | `src/actions/actions.cpp` | `src/actions/spawnPlayerAtXY.hpp (proposed; verify during owning phase)` | pending | TestWorldTravel |
| `startCombat` | function | `src/actions/actions.cpp` | `src/actions/startCombat.hpp (proposed; verify during owning phase)` | pending | TestCombatActions, TestEnemyBehavior, TestCombat |
| `talkAt` | function | `src/actions/actions.cpp` | `src/actions/talkAt.hpp (proposed; verify during owning phase)` | pending | TestWorldTalkAt |
| `toggleEquipInventoryItem` | function | `src/actions/actions.cpp` | `src/actions/toggleEquipInventoryItem.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `toggleManaSlotRune` | function | `src/actions/actions.cpp` | `src/actions/toggleManaSlotRune.hpp (proposed; verify during owning phase)` | pending | model/action suite |
| `travel` | function | `src/actions/actions.cpp` | `src/actions/travel.hpp (proposed; verify during owning phase)` | pending | TestCreateMapInstanceFromTemplate, TestTileTriggers, TestWorldTravel |
| `updateHeldMove` | function | `src/actions/actions.cpp` | `src/actions/updateHeldMove.hpp (proposed; verify during owning phase)` | pending | ImportActions |
| `worldProcessPendingTriggers` | function | `src/actions/world/WorldUpdater.cpp` | `src/state/WorldUpdater.h` | pending | model/action suite |
| `worldUpdate` | function | `src/actions/world/WorldUpdater.cpp` | `src/state/WorldUpdater.h` | pending | TestCameraFollow, TestCombatActions, TestEnemyBehavior |
| `CombatActionContext` | struct | `src/actions/_actions.cppm (inline/declaration-only)` | `src/state/actions/combat/DoCombatAction.hpp` | pending | model/action suite |
| `WorldSetActionModeCtx` | struct | `src/actions/_actions.cppm (inline/declaration-only)` | `src/state/actions/world/WorldSetActionMode.hpp` | pending | model/action suite |

### `src/data/data.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `TileStepSound` | enum | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Tileset.h` | pending | data/model suite |
| `AbilityCostType` | enum-class | `src/game/combat/SpellRules.cpp` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast, TestSpellRules |
| `AbilityType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast, TestSpellRules |
| `AttackClass` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast |
| `CharacterTemplateBehaviorName` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | TestCombatActions, TestCombatZoneCast, TestEnemyBehavior |
| `CombatBehaviorName` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | TestEnemyBehavior |
| `CurrentStatEnum` | enum-class | `src/game/combat/SpellRules.cpp` | `src/model/templates/AbilityTypes.h` | pending | TestSpellRules |
| `DamageType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestLoadAbilityTemplates, TestLoadStatusEffectTemplates |
| `Dice` | enum-class | `src/game/combat/SpellRules.cpp` | `src/model/templates/AbilityTypes.h` | pending | TestLoadAbilityTemplates, TestCombatZoneCast, TestSpellRules |
| `GameEventChildType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestSpecialEventRunner |
| `GameEventType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestWorldTalkAt, TestSpecialEventRunner |
| `ItemType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Items.h` | pending | TestCharacterEquip |
| `ItemUsability` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Items.h` | pending | data/model suite |
| `MapType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | TestCreateMapInstanceFromTemplate |
| `ProjectilePath` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast |
| `ProjectileType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast |
| `RuneType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/RuneTypes.h` | pending | TestLoadSpellTemplates, TestCombatZoneCast, TestSpellRules |
| `StatsEnum` | enum-class | `src/game/combat/SpellRules.cpp` | `src/model/templates/AbilityTypes.h` | pending | TestLoadAbilityTemplates, TestCombatZoneCast, TestSpellRules |
| `StatusActionTargetType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `StatusEffectCondition` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `StatusEventType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `TargetAllegianceSelectType` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `TargetSelectType` | enum-class | `src/game/combat/SpellRules.cpp` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast, TestSpellRules |
| `TileOverlayVisibility` | enum-class | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `abilityCostTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `abilityCostTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `abilityTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `abilityTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `attackClassFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `attackClassToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `characterGetSprite` | function | `src/data/templates.cpp` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `characterGetSpriteAtIndexOffset` | function | `src/data/templates.cpp` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `computeCharacterDerivedStats` | function | `src/data/stats.cpp` | `src/model/stats/CharacterDerivedStats.h` | pending | data/model suite |
| `createRandomId` | function | `src/data/templates.cpp` | `src/model/templates/UtilityTypes.h` | pending | TestListPickUp, TestMinipagePickUp, TestPopupInventoryItem |
| `currentStatEnumFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `currentStatEnumToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `damageTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `damageTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `diceFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `diceToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `getHighScores` | function | `src/lib/hiscore/hiscore.cpp` | `src/lib/hiscore/hiscore.h` | pending | data/model suite |
| `getItemTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `getItemUsabilityFromString` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `getMapTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/Maps.h` | pending | data/model suite |
| `getStringFromItemType` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `getStringFromMapType` | function | `src/data/templates.cpp` | `src/model/templates/Maps.h` | pending | data/model suite |
| `getStringFromTileOverlayVisibility` | function | `src/data/templates.cpp` | `src/model/templates/Maps.h` | pending | data/model suite |
| `getTileOverlayVisibilityFromString` | function | `src/data/templates.cpp` | `src/model/templates/Maps.h` | pending | data/model suite |
| `initCharacterStatsFromTemplate` | function | `src/data/templates.cpp` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `itemTypeIsEquippable` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `itemTypeIsTwoHandedWeapon` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `itemTypeUsesRuneSlots` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `itemTypeUsesWeaponSlots` | function | `src/data/templates.cpp` | `src/model/templates/Items.h` | pending | data/model suite |
| `projectilePathFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `projectilePathToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `projectileTypeFromAnimName` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `projectileTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `projectileTypeHasFacing` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `projectileTypeToAnimBase` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `projectileTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `runeTypeFromIndex` | function | `src/data/templates.cpp` | `src/model/templates/RuneTypes.h` | pending | TestPageMagicSetup |
| `runeTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/RuneTypes.h` | pending | data/model suite |
| `runeTypeIndex` | function | `src/data/templates.cpp` | `src/model/templates/RuneTypes.h` | pending | TestPageMagicSetup |
| `runeTypeToSpriteName` | function | `src/data/templates.cpp` | `src/model/templates/RuneTypes.h` | pending | TestPageMagicSetup |
| `runeTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/RuneTypes.h` | pending | data/model suite |
| `saveHighScores` | function | `src/lib/hiscore/hiscore.cpp` | `src/lib/hiscore/hiscore.h` | pending | data/model suite |
| `statsEnumFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statsEnumToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statusActionTargetTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statusActionTargetTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statusEffectConditionFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statusEffectConditionToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statusEventTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `statusEventTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `targetAllegianceSelectTypeFromString` | function | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `targetAllegianceSelectTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `targetSelectTypeFromString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `targetSelectTypeToString` | function | `src/data/templates.cpp` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `tileOverlayVisibilitySpriteName` | function | `src/data/templates.cpp` | `src/model/templates/Maps.h` | pending | data/model suite |
| `timerStructGetPct` | function | `src/data/templates.cpp` | `src/model/templates/UtilityTypes.h` | pending | data/model suite |
| `timerStructIsComplete` | function | `src/data/templates.cpp` | `src/model/templates/UtilityTypes.h` | pending | data/model suite |
| `timerStructRestart` | function | `src/data/templates.cpp` | `src/model/templates/UtilityTypes.h` | pending | data/model suite |
| `timerStructStart` | function | `src/data/templates.cpp` | `src/model/templates/UtilityTypes.h` | pending | TestStateManagerActions |
| `timerStructUpdate` | function | `src/data/templates.cpp` | `src/model/templates/UtilityTypes.h` | pending | data/model suite |
| `AbilityAttack` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast |
| `AbilityAttackDmg` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestCombatZoneCast |
| `AbilityDamage` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `AbilityDepiction` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `AbilityRestore` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | TestSpellRules |
| `AbilitySave` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `AbilityStatus` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `AbilityTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Abilities.h` | pending | TestLoadAbilityTemplates, TestCombatZoneCast, TestSpellRules |
| `AudioInfo` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | data/model suite |
| `BodyMasteryStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `CarcerMapTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | TestLoadMapTemplates, TestCreateMapInstanceFromTemplate, TestMapPersistence |
| `CarcerMapTileTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `CharacterDerivedStatDefinitions` | struct | `src/data/stats.cpp` | `src/model/stats/CharacterDerivedStatDefinitions.h` | pending | data/model suite |
| `CharacterDerivedStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterDerivedStats.h` | pending | data/model suite |
| `CharacterSkills` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `CharacterStatDefinitions` | struct | `src/data/stats.cpp` | `src/model/stats/CharacterStatDefinitions.h` | pending | data/model suite |
| `CharacterStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | ImportData |
| `CharacterTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | TestLoadCharacterTemplates, TestCombatActions, TestCombatZoneCast |
| `CharacterTemplateBehavior` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateCombat` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateCombatBehavior` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateSound` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateStatus` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateTalk` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `CharacterTemplateVision` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/CharacterTemplate.h` | pending | data/model suite |
| `Choice` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestSpecialEventRunner, TestPageTalkChoice |
| `ChoiceSwitchText` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | data/model suite |
| `CurrentStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `GameEvent` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestLoadSpecialEvents, TestWorldTalkAt, TestSpecialEventIntegration |
| `GameEventChildChoice` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestSpecialEventRunner |
| `GameEventChildEnd` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestSpecialEventRunner |
| `GameEventChildExec` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestSpecialEventRunner |
| `GameEventChildSwitch` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | data/model suite |
| `GenericCombatStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `HiscoreRow` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/lib/hiscore/hiscore.h` | pending | data/model suite |
| `ItemTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Items.h` | pending | TestLoadItemTemplates, TestCharacterEquip, TestCharacterGive |
| `ItemUseAbilityConfig` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Items.h` | pending | data/model suite |
| `ItemWeaponConfig` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Items.h` | pending | data/model suite |
| `MagicMasteryStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `MapCharacterPlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | TestMapPersistence |
| `MapEventTriggerPlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapGridTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/MapGrids.h` | pending | TestLoadMapGridTemplates, TestCombatActions, TestCombatZoneCast |
| `MapItemPlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapLightSourcePlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapMarkerPlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapTileItemEntry` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapTileOverridePlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapTileRef` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `MapTravelTriggerPlacement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `Resistance` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `SpellRuneRequirement` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Spells.h` | pending | TestCombatZoneCast, TestSpellRules |
| `SpellTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/game/combat/SpellRules.h` | pending | TestLoadSpellTemplates, TestCombatZoneCast, TestSpellRules |
| `Stats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `StatusEffectAction` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/StatusEffects.h` | pending | data/model suite |
| `StatusEffectDurationScale` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/StatusEffects.h` | pending | data/model suite |
| `StatusEffectEvent` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/StatusEffects.h` | pending | data/model suite |
| `StatusEffectTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/StatusEffects.h` | pending | TestLoadStatusEffectTemplates |
| `SwitchCase` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | data/model suite |
| `TargetSelectInfo` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `TargetSelectInfoPoint` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/AbilityTypes.h` | pending | data/model suite |
| `TileEventTrigger` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | TestTileTriggers, TestWorldExamineAt |
| `TileLightSource` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | data/model suite |
| `TileMetadata` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Tileset.h` | pending | TestCombatActions, TestCombatZoneCast, TestEnemyBehavior |
| `TileOverrides` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | TestMapVision, TestWorldMovePlayer |
| `TilesetTemplate` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Tileset.h` | pending | TestCombatActions, TestCombatZoneCast, TestEnemyBehavior |
| `TimerStruct` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/UtilityTypes.h` | pending | data/model suite |
| `TrainableCombatStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `TravelTrigger` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/Maps.h` | pending | TestTileTriggers, TestWorldTravel |
| `Variable` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | TestSpecialEventRunner |
| `VariableValue` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | data/model suite |
| `WeaponMasteryStats` | struct | `src/data/data.cppm (inline/declaration-only)` | `src/model/stats/CharacterStats.h` | pending | data/model suite |
| `GameEventChild` | using | `src/data/data.cppm (inline/declaration-only)` | `src/model/templates/SpecialEvents.h` | pending | data/model suite |

### `src/db/_db.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `Database` | class | `src/db/Database.cpp` | `src/db/Database.h` | pending | TestLoadCharacterTemplates, TestLoadMapGridTemplates, TestLoadMapTemplates |
| `loadAbilityTemplates` | function | `src/db/loaders/LoadAbilityTemplates.cpp` | `src/db/loaders/LoadAbilityTemplates.h` | pending | TestLoadAbilityTemplates |
| `loadCharacterTemplates` | function | `src/db/loaders/LoadCharacterTemplates.cpp` | `src/db/loaders/LoadCharacterTemplates.h` | pending | TestLoadCharacterTemplates |
| `loadItemTemplates` | function | `src/db/loaders/LoadItemTemplates.cpp` | `src/db/loaders/LoadItemTemplates.h` | pending | TestLoadItemTemplates |
| `loadMapGridTemplates` | function | `src/db/loaders/LoadMapGridTemplates.cpp` | `src/db/loaders/LoadMapGridTemplates.h` | pending | TestLoadMapGridTemplates |
| `loadMapTemplates` | function | `src/db/loaders/LoadMapTemplates.cpp` | `src/db/loaders/LoadMapTemplates.h` | pending | TestLoadMapTemplates, TestCreateMapInstanceFromTemplate |
| `loadSpecialEvents` | function | `src/db/loaders/LoadSpecialEvents.cpp` | `src/db/loaders/LoadSpecialEvents.h` | pending | TestLoadSpecialEvents, TestSpecialEventIntegration |
| `loadSpellTemplates` | function | `src/db/loaders/LoadSpellTemplates.cpp` | `src/db/loaders/LoadSpellTemplates.h` | pending | TestLoadSpellTemplates |
| `loadStatusEffectTemplates` | function | `src/db/loaders/LoadStatusEffectTemplates.cpp` | `src/db/loaders/LoadStatusEffectTemplates.h` | pending | TestLoadStatusEffectTemplates |
| `loadTilesetTemplates` | function | `src/db/loaders/LoadTilesetTemplates.cpp` | `src/db/loaders/LoadTilesetTemplates.h` | pending | db suite |
| `mapGet` | function | `src/db/_db.cppm (inline/declaration-only)` | `src/db/Database.h` | pending | db suite |
| `parseAbilityAttack` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseAbilityAttackDmg` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseAbilityDamage` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseAbilityDepiction` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseAbilityRestore` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseAbilitySave` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseAbilityStatus` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseCurrentStats` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseDiceArray` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseResistance` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseStats` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |
| `parseTargetSelectInfo` | function | `src/db/loaders/LoadAbilityJson.cpp` | `src/db/loaders/LoadAbilityJson.h` | pending | db suite |

### `src/game/combat/_combat.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `addPartyMembersToCombatMap` | function | `src/game/combat/CombatParty.cpp` | `src/model/Combat.h` | pending | model rules suite |
| `calculateAbilityDamage` | function | `src/game/combat/Damage.cpp` | `src/game/combat/Damage.h` | pending | model rules suite |
| `calculateAttackDamage` | function | `src/game/combat/Damage.cpp` | `src/game/combat/Damage.h` | pending | model rules suite |
| `canEnemySpotPartyAvatar` | function | `src/game/combat/EnemyBehavior.cpp` | `src/game/combat/EnemyBehavior.h` | pending | TestEnemyBehavior |
| `chooseSeekAndMeleeCombatAction` | function | `src/game/combat/EnemyBehavior.cpp` | `src/game/combat/EnemyBehavior.h` | pending | model rules suite |
| `chooseSeekStepToward` | function | `src/game/combat/EnemyBehavior.cpp` | `src/game/combat/EnemyBehavior.h` | pending | model rules suite |
| `getNumDiceSides` | function | `src/game/diceHelpers.cpp` | `src/game/diceHelpers.h` | pending | model rules suite |
| `getProjectileFacingSuffix` | function | `src/game/combat/projectileHelpers.cpp` | `src/game/combat/projectileHelpers.h` | pending | model rules suite |
| `getProjectileTravelDurationMs` | function | `src/game/combat/projectileHelpers.cpp` | `src/game/combat/projectileHelpers.h` | pending | model rules suite |
| `rollDice` | function | `src/game/diceHelpers.cpp` | `src/game/diceHelpers.h` | pending | model rules suite |
| `rollDiceList` | function | `src/game/diceHelpers.cpp` | `src/game/diceHelpers.h` | pending | model rules suite |
| `spellAbilityManaCost` | function | `src/game/combat/SpellRules.cpp` | `src/game/combat/SpellRules.h` | pending | TestPageMagicSetup |
| `updateEnemySpotting` | function | `src/game/combat/EnemyBehavior.cpp` | `src/game/combat/EnemyBehavior.h` | pending | TestEnemyBehavior |
| `CalculatedAbilityDamageResult` | struct | `src/game/combat/_combat.cppm (inline/declaration-only)` | `src/game/combat/Damage.h` | pending | model rules suite |
| `CombatRunner` | struct | `src/game/combat/CombatRunner.cpp` | `src/game/combat/CombatRunner.h` | pending | model rules suite |

### `src/game/inventory/_inventory.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `giveInventoryItem` | function | `src/actions/actions.cpp` | `src/game/inventory/giveInventoryItem.h (proposed; verify during owning phase)` | pending | TestCharacterGive |
| `inventoryWeight` | function | `src/game/inventory/InventoryRules.cpp` | `src/game/inventory/inventoryWeight.h (proposed; verify during owning phase)` | pending | TestPageInventory |
| `toggleEquippedInventoryItem` | function | `src/game/inventory/InventoryRules.cpp` | `src/game/inventory/toggleEquippedInventoryItem.h (proposed; verify during owning phase)` | pending | TestCharacterEquip |

### `src/game/map/TileFields.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `TileFieldType` | enum-class | `src/game/map/TileFields.cppm (inline/declaration-only)` | `src/game/map/TileFields.h` | pending | TestCombatActions, TestMapPersistence, TestTileFieldAging |
| `ageTileFields` | function | `src/game/map/TileFields.cpp` | `src/game/map/TileFields.h` | pending | TestTileFieldAging |
| `tileFieldDefaultMoveDuration` | function | `src/game/map/TileFields.cpp` | `src/game/map/TileFields.h` | pending | TestTileFieldAging |
| `tileFieldExtraSpriteIndex` | function | `src/game/map/TileFields.cpp` | `src/game/map/TileFields.h` | pending | TestTileFields |
| `tileFieldSpriteName` | function | `src/game/map/TileFields.cpp` | `src/game/map/TileFields.h` | pending | TestTileFields |
| `TileField` | struct | `src/game/map/TileFields.cppm (inline/declaration-only)` | `src/game/map/TileFields.h` | pending | TestTileFieldAging, TestTileFields |

### `src/game/map/_map.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `ActiveMapOrchestrator` | class | `src/game/map/ActiveMapOrchestrator.cpp` | `src/game/map/ActiveMapOrchestrator.h` | pending | TestCombat |
| `ageMapInstances` | function | `src/game/map/MapPersistence.cpp` | `src/game/map/ageMapInstances.h (proposed; verify during owning phase)` | pending | TestTileFieldAging |
| `applyCharacterTemplateFromDatabase` | function | `src/game/map/CharacterConstruction.cpp` | `src/game/map/applyCharacterTemplateFromDatabase.h (proposed; verify during owning phase)` | pending | TestCombatActions, TestEnemyBehavior, TestCombat |
| `applyExploredMask` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `applyOpenedDoors` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestMapVision |
| `captureExploredMask` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `captureOpenedDoors` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestMapVision |
| `chebyshevDistance` | function | `src/game/map/TileDistance.cpp` | `src/game/map/TileDistance.h` | pending | TestEnemyBehavior |
| `collectReachableTiles` | function | `src/game/map/MapPathfinding.cpp` | `src/game/map/MapPathfinding.h` | pending | TestMapPickup |
| `collectTilesAt` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | model rules suite |
| `computeCameraFollow` | function | `src/game/map/Camera.cpp` | `src/game/map/Camera.h` | pending | TestCameraFollow, TestWorldMovePlayer |
| `createMapInstances` | function | `src/game/map/MapPersistence.cpp` | `src/game/map/MapPersistence.h` | pending | TestMapPersistence, TestTileFieldAging, TestCombat |
| `doesTileBlockSight` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `findDropCharacterOnActiveMap` | function | `src/game/map/TileTriggers.cpp` | `src/game/map/TileTriggers.h` | pending | model rules suite |
| `findPartyAvatarOnActiveMap` | function | `src/game/map/TileTriggers.cpp` | `src/game/map/TileTriggers.h` | pending | TestWorldMovePlayer, TestWorldSpawnPlayerAtMarker, TestWorldTravel |
| `findTileMetadata` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestWorldMovePlayer |
| `formatExamineMessage` | function | `src/game/map/TileTriggers.cpp` | `src/game/map/TileTriggers.h` | pending | TestTileTriggers |
| `isActiveMapTileContainer` | function | `src/game/map/MapPickup.cpp` | `src/game/map/MapPickup.h` | pending | TestMapPickup |
| `isChebyshevAdjacent` | function | `src/game/map/TileDistance.cpp` | `src/game/map/TileDistance.h` | pending | TestEnemyBehavior |
| `isClosedDoorTile` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestMapVision |
| `isDestinationSeeThrough` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | model rules suite |
| `isDestinationWalkable` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | model rules suite |
| `isInPlayerVisionRange` | function | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `isOpenDoorTile` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestMapVision |
| `isTileCurrentlyVisible` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestWorldExamineAt, TestWorldTalkAt |
| `isTileEffectivelyContainer` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | model rules suite |
| `isTileEffectivelySeeThrough` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `isTileEffectivelyWalkable` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestWorldMovePlayer |
| `isTileInReachableSet` | function | `src/game/map/MapPathfinding.cpp` | `src/game/map/MapPathfinding.h` | pending | TestMapPickup |
| `markMapCharacterDefeated` | function | `src/game/map/MapPersistence.cpp` | `src/game/map/MapPersistence.h` | pending | TestMapPersistence |
| `placePartyAvatarAt` | function | `src/game/map/TileTriggers.cpp` | `src/game/map/TileTriggers.h` | pending | model rules suite |
| `resolveGridIdForMapOrGrid` | function | `src/game/map/MapPersistence.cpp` | `src/game/map/MapPersistence.h` | pending | model rules suite |
| `resolveStepTriggersAt` | function | `src/game/map/TileTriggers.cpp` | `src/game/map/resolveStepTriggersAt.h (proposed; verify during owning phase)` | pending | TestTileTriggers |
| `resolveTileMetadata` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | model rules suite |
| `resolveWorldActionMode` | function | `src/game/map/TileTriggers.cpp` | `src/game/map/resolveWorldActionMode.h (proposed; verify during owning phase)` | pending | model rules suite |
| `tileAtCurrentLayer` | function | `src/game/map/MapWalkability.cpp` | `src/game/map/MapWalkability.h` | pending | TestCombatActions, TestMapPersistence, TestMapPickup |
| `updateActiveMapVisibilityFromParty` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `updateActiveMapVisibilityFromPlayer` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestEnemyBehavior, TestMapVision |
| `updateMapVisibilityFromParty` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | model rules suite |
| `updateMapVisibilityFromPlayer` | function | `src/game/map/MapVision.cpp` | `src/game/map/MapVision.h` | pending | TestMapVision |
| `ActiveMapLoc` | struct | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/ActiveMapOrchestrator.h` | pending | model rules suite |
| `ActiveMapMarker` | struct | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/ActiveMapOrchestrator.h` | pending | TestCombat |
| `CameraPos` | struct | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/Camera.h` | pending | model rules suite |
| `PathTile` | struct | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/MapPathfinding.h` | pending | model rules suite |
| `StepTriggerResult` | struct | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/StepTriggerResult.h (proposed; verify during owning phase)` | pending | model rules suite |
| `MapInstanceStore` | using | `src/game/map/_map.cppm (inline/declaration-only)` | `src/game/map/MapInstanceStore.h (proposed; verify during owning phase)` | pending | model rules suite |

### `src/in3/_in3.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `ConditionEvaluator` | class | `src/in3/ConditionEvaluator.cpp` | `src/runner/ConditionEvaluator.h` | pending | TestConditionalEvaluator |
| `SpecialEventRunner` | class | `src/in3/SpecialEventRunner.cpp` | `src/runner/SpecialEventRunner.h` | pending | TestSpecialEventIntegration, TestSpecialEventRunner |
| `SpecialEventRunnerInterface` | class | `src/in3/SpecialEventRunner.cpp` | `src/runner/SpecialEventRunner.h` | pending | TestSpecialEventIntegration, TestSpecialEventRunner |
| `StringEvaluator` | class | `src/in3/StringEvaluator.cpp` | `src/runner/StringEvaluator.h` | pending | TestStringEvaluator |
| `SpecialEventRunnerInterfaceState` | enum-class | `src/in3/_in3.cppm (inline/declaration-only)` | `src/runner/SpecialEventRunner.h` | pending | TestSpecialEventIntegration |
| `clearTmpStorageKeys` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | TestSpecialEventRunner |
| `getStorage` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | TestStringEvaluator |
| `isFunctionCall` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | runner suite |
| `parseFunctionCall` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | runner suite |
| `setStorage` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | runner suite |
| `splitExecStatements` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | runner suite |
| `splitString` | function | `src/in3/EventRunnerHelpers.cpp` | `src/runner/EventRunnerHelpers.h` | pending | runner suite |
| `trim` | function | `src/in3/EventRunnerHelpers.cpp` | `src/lib/StringUtil.h` | pending | runner suite |
| `ConditionEvaluatorFuncs` | struct | `src/in3/ConditionEvaluator.cpp` | `src/runner/ConditionEvaluator.h` | pending | runner suite |
| `ConditionResult` | struct | `src/in3/_in3.cppm (inline/declaration-only)` | `src/runner/SpecialEventRunner.h` | pending | TestSpecialEventRunner |
| `DisplayTextChoice` | struct | `src/in3/_in3.cppm (inline/declaration-only)` | `src/runner/SpecialEventRunner.h` | pending | runner suite |
| `ErrorInfo` | struct | `src/in3/_in3.cppm (inline/declaration-only)` | `src/runner/SpecialEventRunner.h` | pending | runner suite |
| `FunctionCall` | struct | `src/in3/_in3.cppm (inline/declaration-only)` | `src/runner/EventRunnerHelpers.h` | pending | runner suite |
| `StringEvaluatorFuncs` | struct | `src/in3/StringEvaluator.cpp` | `src/runner/StringEvaluator.h` | pending | runner suite |

### `src/lib/Json.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `Json` | class | `src/lib/Json.cpp` | `src/lib/Json.h` | pending | TestJson |

### `src/lib/StringUtil.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `findFirstNotOf` | function | `src/lib/StringUtil.cppm (inline/declaration-only)` | `src/lib/StringUtil.h` | pending | runner suite |
| `findLastNotOf` | function | `src/lib/StringUtil.cppm (inline/declaration-only)` | `src/lib/StringUtil.h` | pending | runner suite |
| `fromStringView` | function | `src/layers/Layer.cpp` | `src/lib/StringUtil.h` | pending | runner suite |
| `isWhitespace` | function | `src/lib/StringUtil.cppm (inline/declaration-only)` | `src/lib/StringUtil.h` | pending | runner suite |
| `parseFirstTokenAndInt` | function | `src/lib/StringUtil.cppm (inline/declaration-only)` | `src/lib/StringUtil.h` | pending | runner suite |
| `splitByChar` | function | `src/lib/StringUtil.cppm (inline/declaration-only)` | `src/lib/StringUtil.h` | pending | runner suite |
| `splitLines` | function | `src/lib/StringUtil.cppm (inline/declaration-only)` | `src/lib/StringUtil.h` | pending | runner suite |
| `trim` | function | `src/in3/EventRunnerHelpers.cpp` | `src/lib/StringUtil.h` | pending | runner suite |

### `src/model/model.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `CameraMode` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/World.h` | pending | TestCameraFollow, TestWorldActionAim, TestWorldMovePlayer |
| `CharacterEquipmentSlot` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | TestCharacterEquip |
| `CharacterFacing` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterInstance.h` | pending | TestCharacterFacing, TestWorldMovePlayer |
| `CombatActionType` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/Combat.h` | pending | TestCombatActions |
| `EquipItemResult` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | TestCharacterEquip |
| `EquipRuneResult` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | TestSpellRules |
| `GiveItemResult` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | TestCharacterGive |
| `TurnMode` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | TestCombatActions |
| `WorldActionMode` | enum-class | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/World.h` | pending | TestWorldActionAim, TestWorldExamineAt, TestWorldTalkAt |
| `addTileField` | function | `src/model/maps.cpp` | `src/game/map/TileFields.h` | pending | model suite |
| `addTileFieldAt` | function | `src/model/maps.cpp` | `src/game/map/TileFields.h` | pending | TestCombatActions, TestMapPersistence, TestTileFieldAging |
| `ageMapInstanceTileFields` | function | `src/model/maps.cpp` | `src/game/map/TileFields.h` | pending | model suite |
| `agePersistentTileFieldRecords` | function | `src/model/maps.cpp` | `src/game/map/TileFields.h` | pending | model suite |
| `applyCharacterTemplateStartingSpells` | function | `src/model/characters.cpp` | `src/model/templates/CharacterTemplate.h` | pending | model suite |
| `applyCharacterTemplateToInstance` | function | `src/model/characters.cpp` | `src/model/templates/CharacterTemplate.h` | pending | model suite |
| `characterEquipmentSlotAbbrev` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestCharacterEquip |
| `characterGetWeightCapacity` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestPageInventory |
| `characterInstanceIsEnemy` | function | `src/model/characters.cpp` | `src/model/instances/CharacterInstance.h` | pending | model suite |
| `characterPlayerAddItemToInventory` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestCombat, TestLayerInventory, TestLayerPickUp |
| `characterPlayerCanEquipRuneType` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerCountAvailableRunesOfType` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestPageMagicSetup |
| `characterPlayerCountEquippedRunesOfType` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestPageMagicSetup |
| `characterPlayerEquipRuneType` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestSpellRules |
| `characterPlayerGetSprite` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestPageInventory, TestPageMagicSetup |
| `characterPlayerGetSpriteAtIndexOffset` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerIsItemEquippedById` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerRemoveItemFromInventoryById` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerRemoveItemFromInventoryByName` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerReorderInventoryItem` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerSetAvailableRuneCount` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestPageMagicSetup |
| `characterPlayerToggleManaSlotRune` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | TestSpellRules |
| `characterPlayerUnequipOneRuneOfType` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `characterPlayerUnequipRuneFromSlot` | function | `src/model/characters.cpp` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `createCombatFromWorld` | function | `src/model/world.cpp` | `src/model/Combat.h` | pending | model suite |
| `createMapInstanceFromTemplate` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | TestCreateMapInstanceFromTemplate, TestResolveTileToRender |
| `facingFromMoveDelta` | function | `src/model/characters.cpp` | `src/model/instances/CharacterInstance.h` | pending | TestCharacterFacing |
| `findMarkerOnTemplate` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `formatCharacterLogLabel` | function | `src/model/world.cpp` | `src/model/Combat.h` | pending | model suite |
| `getCharacterHp` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `isCharacterAlly` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `isCharacterDefeated` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `isCharacterEnemy` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `isCharacterFacingLeft` | function | `src/model/characters.cpp` | `src/model/instances/CharacterInstance.h` | pending | model suite |
| `isPartyMember` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `mapHasLayer` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | TestCreateMapInstanceFromTemplate, TestResolveTileToRender |
| `mapInstanceFindCharacter` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `mapInstanceGetMinMaxLayer` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `mapInstanceGetTileAt` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `mapInstanceHasLayer` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `mapInstanceTiles` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | TestCombatActions, TestCombatZoneCast, TestCreateMapInstanceFromTemplate |
| `mapLayerAt` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | TestCombatActions, TestCombatZoneCast, TestCreateMapInstanceFromTemplate |
| `mapLayerPtr` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `modifyPartyMemberHp` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `playerFindPartyMemberById` | function | `src/model/characters.cpp` | `src/model/instances/Player.h` | pending | model suite |
| `playerFindPartyMemberByIndex` | function | `src/model/characters.cpp` | `src/model/instances/Player.h` | pending | TestPageMagicSetup |
| `playerFindPartyMemberIndexById` | function | `src/model/characters.cpp` | `src/model/instances/Player.h` | pending | model suite |
| `removeCharacterFromCombatTurnOrder` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `removeExtraPartyMembersFromMap` | function | `src/model/world.cpp` | `src/model/Combat.h` | pending | model suite |
| `resetAllCombatAp` | function | `src/model/world.cpp` | `src/model/Combat.h` | pending | model suite |
| `setCharacterHp` | function | `src/model/combat.cpp` | `src/model/Combat.h` | pending | model suite |
| `tileIndexToXY` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | TestResolveTileToRender, TestWorldSpawnPlayerAtMarker |
| `tileXYToIndex` | function | `src/model/maps.cpp` | `src/model/instances/MapInstance.h` | pending | model suite |
| `updateCharacterFacingFromMove` | function | `src/model/characters.cpp` | `src/model/instances/CharacterInstance.h` | pending | TestCharacterFacing |
| `updateCharacterFacingToward` | function | `src/model/characters.cpp` | `src/model/instances/CharacterInstance.h` | pending | TestCharacterFacing |
| `ActiveMap` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/Combat.h` | pending | TestCombatActions, TestEnemyBehavior, TestMapPersistence |
| `CameraInfo` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/World.h` | pending | model suite |
| `CharacterAvailableRune` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `CharacterInstance` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterInstance.h` | pending | TestCameraFollow, TestCharacterFacing, TestCombatActions |
| `CharacterInventoryItem` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | TestCharacterEquip, TestCharacterGive, TestDropInventoryItem |
| `CharacterPlayer` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | TestCameraFollow, TestCharacterEquip, TestCharacterGive |
| `CharacterPlayerEquipment` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/CharacterPlayer.h` | pending | model suite |
| `Combat` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/Combat.h` | pending | TestCombat |
| `DamageParticle` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/World.h` | pending | model suite |
| `DefeatedCharacterRecord` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | model suite |
| `ExploredMapMask` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | model suite |
| `ItemInstance` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/ItemInstance.h` | pending | TestMapPickup, TestTileTriggers, TestListPickUp |
| `MapInstance` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | TestCameraFollow, TestCombatActions, TestCombatZoneCast |
| `OpenedDoorRecord` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | model suite |
| `PersistentMapState` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/Combat.h` | pending | model suite |
| `PersistentTileFieldRecord` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/game/map/TileFields.h` | pending | model suite |
| `Player` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/Player.h` | pending | TestEnemyBehavior, TestMapVision, ImportModel |
| `SpellTargetInfo` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/Combat.h` | pending | model suite |
| `TileInstance` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/TileInstance.h` | pending | TestCombatActions, TestCombatZoneCast, TestEnemyBehavior |
| `TileXY` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | TestWorldActionAim, TestWorldExamineAt, TestWorldTalkAt |
| `World` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/World.h` | pending | TestWorldTravel, ImportModel, TestSectionScrollable |
| `WorldProjectile` | struct | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/World.h` | pending | model suite |
| `TileLayerMap` | using | `src/model/model.cppm (inline/declaration-only)` | `src/model/instances/MapInstance.h` | pending | model suite |

### `src/modules/_carcer.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `runCarcer` | function | `src/main.cpp` | `src/app/runCarcer.h (proposed; verify during owning phase)` | pending | ImportCarcer |

### `src/state/_State.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `AbstractAction` | class | `src/state/_State.cppm (inline/declaration-only)` | `src/state/AbstractAction.h` | pending | TestStateManagerActions |
| `ActionBus` | class | `src/state/ActionBus.cpp` | `src/state/ActionBus.h` | pending | model/state suite |
| `DatabaseInterface` | class | `src/state/DatabaseInterface.cpp` | `src/state/DatabaseInterface.h` | pending | TestCombatActions, TestCombatZoneCast, TestDropInventoryItem |
| `LayerManagerInterface` | class | `src/state/LayerManagerInterface.cpp` | `src/state/LayerManagerInterface.h` | pending | TestCombat, TestLayerInventory, TestLayerPickUp |
| `StateManager` | class | `src/state/StateManager.cpp` | `src/state/StateManager.h` | pending | TestCameraFollow, TestCombatActions, TestCombatZoneCast |
| `StateManagerInterface` | class | `src/state/StateManager.cpp` | `src/state/StateManagerInterface.h` | pending | TestCombatActions, TestCombatZoneCast, TestEnemyBehavior |
| `ActionEvent` | enum-class | `src/state/_State.cppm (inline/declaration-only)` | `src/state/ActionEvent.h (proposed; verify during owning phase)` | pending | TestStateManagerActions, TestPageMagicSetup |
| `LayerId` | enum-class | `src/ui/layers.cpp` | `src/state/LayerId.h (proposed; verify during owning phase)` | pending | model/state suite |
| `UiFloatingNotificationType` | enum-class | `src/state/_State.cppm (inline/declaration-only)` | `src/state/State.h` | pending | TestFloatingNotificationSection |
| `WorldActionType` | enum-class | `src/state/_State.cppm (inline/declaration-only)` | `src/state/WorldActions.h` | pending | TestButtonWorldAction, TestInGameLayout |
| `layerIdFromString` | function | `src/state/_State.cppm (inline/declaration-only)` | `src/state/layerIdFromString.h (proposed; verify during owning phase)` | pending | model/state suite |
| `layerIdString` | function | `src/state/_State.cppm (inline/declaration-only)` | `src/state/layerIdString.h (proposed; verify during owning phase)` | pending | model/state suite |
| `pushLayerRequest` | function | `src/state/_State.cppm (inline/declaration-only)` | `src/state/pushLayerRequest.h (proposed; verify during owning phase)` | pending | model/state suite |
| `removeLayerRequest` | function | `src/state/_State.cppm (inline/declaration-only)` | `src/state/removeLayerRequest.h (proposed; verify during owning phase)` | pending | model/state suite |
| `updateUiState` | function | `src/state/UiManager.cpp` | `src/state/updateUiState.h (proposed; verify during owning phase)` | pending | model/state suite |
| `ActionData` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/StateManager.h` | pending | model/state suite |
| `AsyncAction` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/StateManager.h` | pending | model/state suite |
| `HeldMove` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/State.h` | pending | ImportActions |
| `LayerRequest` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/LayerRequest.h (proposed; verify during owning phase)` | pending | model/state suite |
| `State` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/State.h` | pending | TestCombatActions, TestCombatZoneCast, TestDropInventoryItem |
| `Triggers` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/Triggers.h` | pending | model/state suite |
| `UiFloatingNotification` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/State.h` | pending | TestStateManagerActions |
| `UiState` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/State.h` | pending | model/state suite |
| `UserSettings` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/State.h` | pending | model/state suite |
| `WorldActionUiState` | struct | `src/state/_State.cppm (inline/declaration-only)` | `src/state/WorldActions.h` | pending | TestInGameLayout |

### `src/ui/_core.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `StateInterface` | class | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/UiElement.h` | pending | UI compile-only suite |
| `UiElement` | class | `src/ui/UiElement.cpp` | `src/ui/UiElement.h` | pending | TestBorderInGameNarrow, TestBorderInGameWide, TestBorderModalSmall |
| `UiEventObserver` | class | `src/ui/UiElement.cpp` | `src/ui/UiElement.h` | pending | TestConfirmModal, TestFloatingNotificationSection, TestInGameTitleBar |
| `BaseFontConfig` | enum-class | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/TextStyle.h` | pending | TestHorizontalSlider, TestSectionScrollable, TestTextParagraph |
| `FontFamily` | enum-class | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/TextStyle.h` | pending | UI compile-only suite |
| `TextAlign` | enum-class | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/TextStyle.h` | pending | TestHorizontalSlider, TestSectionScrollable, TestVerticalList |
| `applyFontScale` | function | `src/ui/FontScale.cpp` | `src/ui/FontScale.h` | pending | TestSystemFontScale |
| `isInBounds` | function | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/uiUtils.h` | pending | UI compile-only suite |
| `isInBoundsScaled` | function | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/uiUtils.h` | pending | UI compile-only suite |
| `mapFontSizeToPixels` | function | `src/ui/FontScale.cpp` | `src/ui/FontScale.h` | pending | UI compile-only suite |
| `mapPixelsToFontSize` | function | `src/ui/FontScale.cpp` | `src/ui/FontScale.h` | pending | UI compile-only suite |
| `setBaseFontConfig` | function | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/TextStyle.h` | pending | TestHorizontalSlider, TestSectionScrollable, TestTextParagraph |
| `BaseStyle` | struct | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/UiElement.h` | pending | ImportUiCore |
| `Colors` | struct | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/colors.h` | pending | ImportUiCore, TestButtonTextWrap, TestHorizontalSlider |
| `TextFontProps` | struct | `src/ui/_core.cppm (inline/declaration-only)` | `src/ui/TextStyle.h` | pending | ImportUiCore, TestHorizontalSlider, TestSectionScrollable |

### `src/ui/_layers.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `Layer` | class | `src/ui/layers.cpp` | `src/layers/Layer.h` | pending | TestMinipageCharacterSheet, TestMinipageEvent, TestPageCharacter |
| `LayerManager` | class | `src/layers/LayerManager.cpp` | `src/layers/LayerManager.h` | pending | TestCombat, TestLayerInventory, TestLayerPickUp |
| `LayerState` | enum-class | `src/layers/Layer.cpp` | `src/layers/Layer.h` | pending | ImportUiLayers |
| `createInventoryLayer` | function | `src/ui/layers.cpp` | `src/layers/createInventoryLayer.h (proposed; verify during owning phase)` | pending | TestLayerInventory |
| `createPickUpLayer` | function | `src/ui/layers.cpp` | `src/layers/createPickUpLayer.h (proposed; verify during owning phase)` | pending | TestLayerPickUp |
| `createWorldLayer` | function | `src/ui/layers.cpp` | `src/layers/createWorldLayer.h (proposed; verify during owning phase)` | pending | ImportUiLayers, TestCombat, TestLayerWorld |

### `src/ui/_screens.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `InGameLayout` | class | `src/ui/screens/layouts.cpp` | `src/ui/layouts/InGameLayout.h` | pending | TestInGameLayout |
| `KeyboardHeldScroll` | class | `src/ui/KeyboardHeldScroll.cpp` | `src/ui/KeyboardHeldScroll.h` | pending | UI compile-only suite |
| `MinipageCharacterSheet` | class | `src/ui/screens/overlays.cpp` | `src/ui/minipages/MinipageCharacterSheet.h` | pending | TestMinipageCharacterSheet |
| `MinipageEquipRunes` | class | `src/ui/screens/overlays.cpp` | `src/ui/minipages/MinipageEquipRunes.h` | pending | UI compile-only suite |
| `MinipageEvent` | class | `src/ui/screens/overlays.cpp` | `src/ui/minipages/MinipageEvent.h` | pending | TestMinipageEvent |
| `MinipagePickUp` | class | `src/ui/screens/overlays.cpp` | `src/ui/minipages/MinipagePickUp.h` | pending | TestMinipagePickUp |
| `MinipageSpellCast` | class | `src/ui/screens/overlays.cpp` | `src/ui/minipages/MinipageSpellCast.h` | pending | UI compile-only suite |
| `ModalSmall` | class | `src/ui/screens/layouts.cpp` | `src/ui/layouts/ModalSmall.h` | pending | TestModalSmall |
| `ModalStandard` | class | `src/ui/screens/layouts.cpp` | `src/ui/layouts/ModalStandard.h` | pending | TestModalStandard |
| `ObserverAdjustEquippedRune` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverAdjustEquippedRune.hpp` | pending | UI compile-only suite |
| `ObserverCancelEquipRunes` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverCancelEquipRunes.hpp` | pending | UI compile-only suite |
| `ObserverCancelWorldActionMode` | class | `src/ui/screens/layouts.cpp` | `src/ui/observers/ObserverCancelWorldActionMode.hpp` | pending | UI compile-only suite |
| `ObserverCommitEquipRunes` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverCommitEquipRunes.hpp` | pending | UI compile-only suite |
| `ObserverDropInventoryItem` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverDropInventoryItem.hpp` | pending | UI compile-only suite |
| `ObserverGiveInventoryItem` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverGiveInventoryItem.hpp` | pending | UI compile-only suite |
| `ObserverRemoveLayer` | class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/observers/ObserverRemoveLayer.hpp` | pending | UI compile-only suite |
| `ObserverSetSelectedPartyMemberId` | class | `src/ui/screens/layouts.cpp` | `src/ui/observers/ObserverSetSelectedPartyMemberId.hpp` | pending | UI compile-only suite |
| `ObserverSetSpellReady` | class | `src/ui/screens/pages.cpp` | `src/ui/observers/ObserverSetSpellReady.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerDropContext` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverShowLayerDropContext.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerEquipRunes` | class | `src/ui/screens/pages.cpp` | `src/ui/observers/ObserverShowLayerEquipRunes.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerGiveContext` | class | `src/ui/screens/overlays.cpp` | `src/ui/observers/ObserverShowLayerGiveContext.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerPopupText` | class | `src/ui/screens/pages.cpp` | `src/ui/observers/ObserverShowLayerPopupText.hpp` | pending | UI compile-only suite |
| `ObserverSpecialEventChoice` | class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/observers/ObserverSpecialEventChoice.hpp` | pending | UI compile-only suite |
| `ObserverSpecialEventContinue` | class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/observers/ObserverSpecialEventContinue.hpp` | pending | UI compile-only suite |
| `ObserverToggleManaSlotRune` | class | `src/ui/screens/pages.cpp` | `src/ui/observers/ObserverToggleManaSlotRune.hpp` | pending | UI compile-only suite |
| `ObserverWorldAction` | class | `src/ui/screens/layouts.cpp` | `src/ui/observers/ObserverWorldAction.hpp` | pending | UI compile-only suite |
| `PageCharacter` | class | `src/ui/screens/pages.cpp` | `src/ui/pages/PageCharacter.h` | pending | TestPageCharacter |
| `PageInventory` | class | `src/ui/screens/pages.cpp` | `src/ui/pages/PageInventory.h` | pending | TestPageInventory |
| `PageMagicSetup` | class | `src/ui/screens/pages.cpp` | `src/ui/pages/PageMagicSetup.h` | pending | TestPageMagicSetup |
| `PageModalEvent` | class | `src/ui/screens/pages.cpp` | `src/ui/pages/PageModalEvent.h` | pending | UI compile-only suite |
| `PageTalkChoice` | class | `src/ui/screens/pages.cpp` | `src/ui/pages/PageTalkChoice.h` | pending | TestPageTalkChoice |
| `PopupDropConfirm` | class | `src/ui/screens/overlays.cpp` | `src/ui/popups/PopupDropConfirm.h` | pending | UI compile-only suite |
| `PopupGive` | class | `src/ui/screens/overlays.cpp` | `src/ui/popups/PopupGive.h` | pending | UI compile-only suite |
| `PopupInventoryItem` | class | `src/ui/screens/overlays.cpp` | `src/ui/popups/PopupInventoryItem.h` | pending | TestPopupInventoryItem |
| `PopupPickupItem` | class | `src/ui/screens/overlays.cpp` | `src/ui/popups/PopupPickupItem.h` | pending | TestPopupPickupItem |
| `PopupSpellInfo` | class | `src/ui/screens/overlays.cpp` | `src/ui/popups/PopupSpellInfo.h` | pending | UI compile-only suite |
| `PopupOrientation` | enum | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupInventoryItem.h` | pending | TestPopupInventoryItem, TestPopupPickupItem |
| `HeldScrollDirection` | enum-class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/HeldScrollDirection.h (proposed; verify during owning phase)` | pending | UI compile-only suite |
| `InGameBorderType` | enum-class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/layouts/InGameLayout.h` | pending | TestInGameLayout |
| `LayoutFit` | enum-class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/helpers/modalLayoutFit.h` | pending | TestModalSmall |
| `ModalSizeClass` | enum-class | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/helpers/modalLayoutFit.h` | pending | UI compile-only suite |
| `activateWorldAction` | function | `src/ui/helpers/worldActions.cpp` | `src/ui/helpers/worldActions.h` | pending | UI compile-only suite |
| `cancelCurrentWorldActionMode` | function | `src/ui/helpers/worldActions.cpp` | `src/ui/helpers/worldActions.h` | pending | UI compile-only suite |
| `computeCappedCenteredRect` | function | `src/ui/helpers/modalLayoutFit.cpp` | `src/ui/helpers/modalLayoutFit.h` | pending | UI compile-only suite |
| `getMoveDeltaForKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `getPartyMemberIndexFromKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `getPickUpItemIndexFromKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `isCancelActionKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `isCombatWaitKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `isConfirmActionKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `isOpenMagicSetupKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `isOpenSpellCastKey` | function | `src/ui/helpers/keyboardShortcuts.cpp` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `setHeldMoveActive` | function | `src/ui/helpers/worldActions.cpp` | `src/ui/helpers/worldActions.h` | pending | UI compile-only suite |
| `showMagicSetupLayer` | function | `src/ui/helpers/worldActions.cpp` | `src/ui/helpers/worldActions.h` | pending | UI compile-only suite |
| `showSpellCastLayer` | function | `src/ui/helpers/worldActions.cpp` | `src/ui/helpers/worldActions.h` | pending | UI compile-only suite |
| `syncHostStyleToCappedCentered` | function | `src/ui/helpers/modalLayoutFit.cpp` | `src/ui/helpers/modalLayoutFit.h` | pending | UI compile-only suite |
| `InGameLayoutProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/layouts/InGameLayout.h` | pending | TestInGameLayout |
| `LayoutRect` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/helpers/modalLayoutFit.h` | pending | ImportUiScreens |
| `MinipageCharacterSheetProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageCharacterSheet.h` | pending | TestMinipageCharacterSheet |
| `MinipageEquipRunesProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageEquipRunes.h` | pending | UI compile-only suite |
| `MinipageEquipRunesRow` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageEquipRunes.h` | pending | UI compile-only suite |
| `MinipageEquipRunesSlot` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageEquipRunes.h` | pending | UI compile-only suite |
| `MinipageEventProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageEvent.h` | pending | TestMinipageEvent |
| `MinipagePickUpProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipagePickUp.h` | pending | UI compile-only suite |
| `MinipageSpellCastProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageSpellCast.h` | pending | UI compile-only suite |
| `MinipageSpellCastSpell` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/minipages/MinipageSpellCast.h` | pending | UI compile-only suite |
| `ModalSmallProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/layouts/ModalSmall.h` | pending | TestModalSmall |
| `ModalStandardProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/layouts/ModalStandard.h` | pending | TestModalStandard |
| `MoveDelta` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/helpers/keyboardShortcuts.h` | pending | UI compile-only suite |
| `PageCharacterProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageCharacter.h` | pending | TestPageCharacter |
| `PageCharacterStatRowEntry` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageCharacter.h` | pending | UI compile-only suite |
| `PageCharacterStatRowSectionArgs` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageCharacter.h` | pending | UI compile-only suite |
| `PageInventoryPartyMember` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageInventory.h` | pending | UI compile-only suite |
| `PageInventoryProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageInventory.h` | pending | TestPageInventory |
| `PageMagicSetupElementCount` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageMagicSetup.h` | pending | TestPageMagicSetup |
| `PageMagicSetupPartyMember` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageMagicSetup.h` | pending | UI compile-only suite |
| `PageMagicSetupProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageMagicSetup.h` | pending | TestPageMagicSetup |
| `PageMagicSetupRuneSlot` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageMagicSetup.h` | pending | TestPageMagicSetup |
| `PageMagicSetupSpellEntry` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageMagicSetup.h` | pending | TestPageMagicSetup |
| `PageModalEventProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageModalEvent.h` | pending | UI compile-only suite |
| `PageTalkChoiceItem` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageTalkChoice.h` | pending | UI compile-only suite |
| `PageTalkChoiceProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/pages/PageTalkChoice.h` | pending | TestPageTalkChoice |
| `PopupDropConfirmProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupDropConfirm.h` | pending | UI compile-only suite |
| `PopupGivePartyMember` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupGive.h` | pending | UI compile-only suite |
| `PopupGiveProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupGive.h` | pending | UI compile-only suite |
| `PopupInventoryItemProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupInventoryItem.h` | pending | TestPopupInventoryItem |
| `PopupPickupItemProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupPickupItem.h` | pending | TestPopupPickupItem |
| `PopupSpellInfoProps` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupSpellInfo.h` | pending | UI compile-only suite |
| `PopupSpellInfoRuneReq` | struct | `src/ui/_screens.cppm (inline/declaration-only)` | `src/ui/popups/PopupSpellInfo.h` | pending | UI compile-only suite |

### `src/ui/_widget_composites.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `BorderInGameNarrow` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/borders/BorderInGameNarrow.h` | pending | TestBorderInGameNarrow |
| `BorderInGameWide` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/borders/BorderInGameWide.h` | pending | TestBorderInGameWide |
| `BorderModalSmall` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/borders/BorderModalSmall.h` | pending | TestBorderModalSmall |
| `BorderModalStandard` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/borders/BorderModalStandard.h` | pending | TestBorderModalStandard |
| `ConfirmModal` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/ConfirmModal.h` | pending | TestConfirmModal |
| `FloatingNotification` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/FloatingNotification.h` | pending | UI compile-only suite |
| `FloatingNotificationSection` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/FloatingNotificationSection.h` | pending | TestFloatingNotificationSection |
| `ListChCompactInfoHorizontal` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/lists/ListChCompactInfoHorizontal.h` | pending | TestListChCompactInfoHorizontal |
| `ListChCompactInfoVertical` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/lists/ListChCompactInfoVertical.h` | pending | TestListChCompactInfoVertical |
| `ListInventory` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/lists/ListInventory.h` | pending | TestListInventory |
| `ListMagicSpells` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/lists/ListMagicSpells.h` | pending | UI compile-only suite |
| `ListPickUp` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/lists/ListPickUp.h` | pending | TestListPickUp |
| `ObserverInventorySelectItem` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverInventorySelectItem.hpp` | pending | UI compile-only suite |
| `ObserverPickUpItem` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverPickUpItem.hpp` | pending | UI compile-only suite |
| `ObserverReorderInventoryItem` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverReorderInventoryItem.hpp` | pending | UI compile-only suite |
| `ObserverSelectSpellCast` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverSelectSpellCast.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerInventoryContext` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverShowLayerInventoryContext.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerPickUpContext` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverShowLayerPickUpContext.hpp` | pending | UI compile-only suite |
| `ObserverShowLayerSpellInfo` | class | `src/ui/widgets/composites.cpp` | `src/ui/observers/ObserverShowLayerSpellInfo.hpp` | pending | UI compile-only suite |
| `TouchMovePad` | class | `src/ui/widgets/composites.cpp` | `src/ui/components/TouchMovePad.h` | pending | TestTouchMovePad |
| `BorderInGameNarrowProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/borders/BorderInGameNarrow.h` | pending | TestBorderInGameNarrow |
| `BorderInGameWideProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/borders/BorderInGameWide.h` | pending | TestBorderInGameWide |
| `BorderModalSmallProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/borders/BorderModalSmall.h` | pending | TestBorderModalSmall, TestBorderModalStandard |
| `ConfirmModalProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/ConfirmModal.h` | pending | TestConfirmModal |
| `FloatingNotificationProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/FloatingNotification.h` | pending | UI compile-only suite |
| `FloatingNotificationSectionProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/FloatingNotificationSection.h` | pending | UI compile-only suite |
| `ListChCompactInfoHorizontalProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListChCompactInfoHorizontal.h` | pending | TestListChCompactInfoHorizontal |
| `ListChCompactInfoVerticalProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListChCompactInfoVertical.h` | pending | TestListChCompactInfoVertical |
| `ListInventoryProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListInventory.h` | pending | UI compile-only suite |
| `ListInventoryPropsItem` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListInventory.h` | pending | UI compile-only suite |
| `ListMagicSpellsProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListMagicSpells.h` | pending | UI compile-only suite |
| `ListMagicSpellsPropsSpell` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListMagicSpells.h` | pending | UI compile-only suite |
| `ListPickUpProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListPickUp.h` | pending | TestListPickUp |
| `ListPickUpPropsItem` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/lists/ListPickUp.h` | pending | UI compile-only suite |
| `TouchMovePadProps` | struct | `src/ui/_widget_composites.cppm (inline/declaration-only)` | `src/ui/components/TouchMovePad.h` | pending | TestTouchMovePad |

### `src/ui/_widget_foundation.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `BorderDropShadow` | class | `src/ui/widgets/foundation_views.cpp` | `src/ui/components/borders/BorderDropShadow.h` | pending | TestSection |
| `BorderInGame` | class | `src/ui/widgets/foundation_views.cpp` | `src/ui/components/borders/BorderInGame.h` | pending | UI compile-only suite |
| `ButtonClose` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonClose.h` | pending | TestButtonModal |
| `ButtonGroup` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonGroup.h` | pending | TestButtonGroup |
| `ButtonIcon` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonIcon.h` | pending | UI compile-only suite |
| `ButtonList` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonList.h` | pending | UI compile-only suite |
| `ButtonModal` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonModal.h` | pending | TestFloatingNotificationSection, TestButtonModal, TestTextBanner |
| `ButtonMove` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonMove.h` | pending | UI compile-only suite |
| `ButtonScroll` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonScroll.h` | pending | TestButtonModal |
| `ButtonSprite` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonSprite.h` | pending | UI compile-only suite |
| `ButtonTextWrap` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonTextWrap.h` | pending | TestButtonTextWrap |
| `ButtonWorldAction` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/buttons/ButtonWorldAction.h` | pending | TestButtonWorldAction |
| `ChCompactInfo` | class | `src/ui/components/ChCompactInfo.cpp` | `src/ui/components/ChCompactInfo.h` | pending | TestChCompactInfo |
| `HorizontalList` | class | `src/ui/widgets/primitives.cpp` | `src/ui/elements/HorizontalList.h` | pending | UI compile-only suite |
| `HorizontalSlider` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/HorizontalSlider.h` | pending | TestHorizontalSlider |
| `OutsetRectangle` | class | `src/ui/widgets/primitives.cpp` | `src/ui/elements/OutsetRectangle.h` | pending | TestOutsetRectangle |
| `Quad` | class | `src/ui/widgets/primitives.cpp` | `src/ui/elements/Quad.h` | pending | TestQuad, TestSectionScrollable, TestTextParagraph |
| `SectionScrollable` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/SectionScrollable.h` | pending | TestSectionScrollable |
| `SpriteElement` | class | `src/ui/widgets/primitives.cpp` | `src/ui/elements/SpriteElement.h` | pending | UI compile-only suite |
| `TextBanner` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/TextBanner.h` | pending | TestTextBanner |
| `TextLine` | class | `src/ui/widgets/primitives.cpp` | `src/ui/elements/TextLine.h` | pending | TestHorizontalSlider, TestSectionScrollable, TestVerticalList |
| `TextParagraph` | class | `src/ui/widgets/controls.cpp` | `src/ui/elements/TextParagraph.h` | pending | TestTextParagraph |
| `TiledOverlay` | class | `src/ui/widgets/foundation_views.cpp` | `src/ui/components/TiledOverlay.h` | pending | UI compile-only suite |
| `VerticalList` | class | `src/ui/widgets/primitives.cpp` | `src/ui/elements/VerticalList.h` | pending | TestVerticalList |
| `ButtonGroupAlignment` | enum-class | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonGroup.h` | pending | TestButtonGroup |
| `ButtonGroupButtonType` | enum-class | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonGroup.h` | pending | TestButtonGroup |
| `CloseType` | enum-class | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonClose.h` | pending | TestButtonModal |
| `MoveDirection` | enum-class | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonMove.h` | pending | UI compile-only suite |
| `ScrollDirection` | enum-class | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/KeyboardHeldScroll.h` | pending | TestButtonModal |
| `TextBannerCorner` | enum-class | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextBanner.h` | pending | TestTextBanner |
| `BorderDropShadowProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/components/borders/BorderDropShadow.h` | pending | TestSection |
| `BorderInGameProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/components/borders/BorderInGame.h` | pending | UI compile-only suite |
| `ButtonCloseProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonClose.h` | pending | TestButtonModal |
| `ButtonGroupButtonProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonGroup.h` | pending | TestButtonGroup |
| `ButtonGroupProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonGroup.h` | pending | TestButtonGroup |
| `ButtonIconProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonIcon.h` | pending | UI compile-only suite |
| `ButtonListProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonList.h` | pending | UI compile-only suite |
| `ButtonModalProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonModal.h` | pending | TestFloatingNotificationSection, TestButtonModal, TestTextBanner |
| `ButtonMoveProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonMove.h` | pending | UI compile-only suite |
| `ButtonScrollProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonScroll.h` | pending | TestButtonModal |
| `ButtonSpriteProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonSprite.h` | pending | UI compile-only suite |
| `ButtonTextWrapProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonTextWrap.h` | pending | TestButtonTextWrap |
| `ButtonWorldActionMapping` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonWorldAction.h` | pending | UI compile-only suite |
| `ButtonWorldActionProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/buttons/ButtonWorldAction.h` | pending | TestButtonWorldAction |
| `ChCompactInfoProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/components/ChCompactInfo.h` | pending | ImportUiWidgets, TestChCompactInfo, TestListChCompactInfoHorizontal |
| `HorizontalListProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/HorizontalList.h` | pending | UI compile-only suite |
| `HorizontalSliderProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/HorizontalSlider.h` | pending | UI compile-only suite |
| `OutsetRectangleProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/OutsetRectangle.h` | pending | TestOutsetRectangle |
| `QuadProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/Quad.h` | pending | ImportUiWidgets, TestQuad, TestSectionScrollable |
| `SectionScrollableProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/SectionScrollable.h` | pending | TestSectionScrollable |
| `SpriteElementProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/SpriteElement.h` | pending | UI compile-only suite |
| `TextBannerProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextBanner.h` | pending | TestTextBanner |
| `TextBlock` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextLine.h` | pending | TestSectionScrollable, TestTextParagraph, TestModalStandard |
| `TextLineProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextLine.h` | pending | TestSectionScrollable, TestVerticalList, TestModalSmall |
| `TextLineRenderTextParams` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextLine.h` | pending | UI compile-only suite |
| `TextParagraphGeneratedBlock` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextParagraph.h` | pending | UI compile-only suite |
| `TextParagraphProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/TextParagraph.h` | pending | TestTextParagraph |
| `TiledOverlayProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/components/TiledOverlay.h` | pending | UI compile-only suite |
| `VerticalListProps` | struct | `src/ui/_widget_foundation.cppm (inline/declaration-only)` | `src/ui/elements/VerticalList.h` | pending | TestVerticalList |

### `src/ui/_widget_views.cppm`

| Symbol | Kind | Current implementation | Target header/source | Status | Relevant tests |
|---|---|---|---|---|---|
| `InGameTitleBar` | class | `src/ui/widgets/views.cpp` | `src/ui/components/InGameTitleBar.h` | pending | TestInGameTitleBar, TestInGameLayout |
| `ItemInfo` | class | `src/ui/widgets/views.cpp` | `src/ui/components/ItemInfo.h` | pending | UI compile-only suite |
| `MapView` | class | `src/ui/widgets/views.cpp` | `src/ui/components/MapView.h` | pending | UI compile-only suite |
| `ObserverSetCurrentPartyMember` | class | `src/ui/widgets/views.cpp` | `src/ui/observers/ObserverSetCurrentPartyMember.hpp` | pending | UI compile-only suite |
| `ObserverSetCurrentPartyMemberInventory` | class | `src/ui/widgets/views.cpp` | `src/ui/observers/ObserverSetCurrentPartyMemberInventory.hpp` | pending | UI compile-only suite |
| `ObserverSetCurrentPartyMemberMagic` | class | `src/ui/widgets/views.cpp` | `src/ui/observers/ObserverSetCurrentPartyMemberMagic.hpp` | pending | UI compile-only suite |
| `ObserverUpdateCurrentPartyMember` | class | `src/ui/widgets/views.cpp` | `src/ui/observers/ObserverUpdateCurrentPartyMember.hpp` | pending | UI compile-only suite |
| `PartyMemberIconSelector` | class | `src/ui/widgets/views.cpp` | `src/ui/components/PartyMemberIconSelector.h` | pending | UI compile-only suite |
| `PartyMemberSwitcher` | class | `src/ui/widgets/views.cpp` | `src/ui/components/PartyMemberSwitcher.h` | pending | TestPartyMemberSwitcher |
| `PartyMemberIconSelectorTarget` | enum-class | `src/ui/_widget_views.cppm (inline/declaration-only)` | `src/ui/components/PartyMemberIconSelector.h` | pending | UI compile-only suite |
| `InGameTitleBarProps` | struct | `src/ui/_widget_views.cppm (inline/declaration-only)` | `src/ui/components/InGameTitleBar.h` | pending | TestInGameTitleBar, TestInGameLayout |
| `ItemInfoProps` | struct | `src/ui/_widget_views.cppm (inline/declaration-only)` | `src/ui/components/ItemInfo.h` | pending | UI compile-only suite |
| `MapViewProps` | struct | `src/ui/_widget_views.cppm (inline/declaration-only)` | `src/ui/components/MapView.h` | pending | UI compile-only suite |
| `PartyMemberIconSelectorProps` | struct | `src/ui/_widget_views.cppm (inline/declaration-only)` | `src/ui/components/PartyMemberIconSelector.h` | pending | UI compile-only suite |
| `PartyMemberSwitcherProps` | struct | `src/ui/_widget_views.cppm (inline/declaration-only)` | `src/ui/components/PartyMemberSwitcher.h` | pending | TestPartyMemberSwitcher |

### `src/ui/_widgets.cppm`

Aggregator only; no independently declared public type/function. Status: `pending` until all re-exported owning interfaces are verified.

## Header-reference production path inventory

Every tracked production `.h`, `.hpp`, and `.cpp` path at `fe50a90`, grouped by top-level subsystem. These are layout candidates, not an instruction to restore obsolete files blindly.

### app

```text
src/main.cpp
```

### db

```text
src/db/Database.cpp
src/db/Database.h
src/db/loaders/LoadAbilityJson.cpp
src/db/loaders/LoadAbilityJson.h
src/db/loaders/LoadAbilityTemplates.cpp
src/db/loaders/LoadAbilityTemplates.h
src/db/loaders/LoadCharacterTemplates.cpp
src/db/loaders/LoadCharacterTemplates.h
src/db/loaders/LoadItemTemplates.cpp
src/db/loaders/LoadItemTemplates.h
src/db/loaders/LoadMapGridTemplates.cpp
src/db/loaders/LoadMapGridTemplates.h
src/db/loaders/LoadMapTemplates.cpp
src/db/loaders/LoadMapTemplates.h
src/db/loaders/LoadSpecialEvents.cpp
src/db/loaders/LoadSpecialEvents.h
src/db/loaders/LoadSpellTemplates.cpp
src/db/loaders/LoadSpellTemplates.h
src/db/loaders/LoadStatusEffectTemplates.cpp
src/db/loaders/LoadStatusEffectTemplates.h
src/db/loaders/LoadTilesetTemplates.cpp
src/db/loaders/LoadTilesetTemplates.h
```

### game

```text
src/game/combat/CombatRunner.cpp
src/game/combat/CombatRunner.h
src/game/combat/Damage.cpp
src/game/combat/Damage.h
src/game/combat/EnemyBehavior.cpp
src/game/combat/EnemyBehavior.h
src/game/combat/SpellRules.cpp
src/game/combat/SpellRules.h
src/game/combat/projectileHelpers.cpp
src/game/combat/projectileHelpers.h
src/game/diceHelpers.cpp
src/game/diceHelpers.h
src/game/map/ActiveMapOrchestrator.cpp
src/game/map/ActiveMapOrchestrator.h
src/game/map/Camera.cpp
src/game/map/Camera.h
src/game/map/MapPathfinding.cpp
src/game/map/MapPathfinding.h
src/game/map/MapPersistence.cpp
src/game/map/MapPersistence.h
src/game/map/MapPickup.cpp
src/game/map/MapPickup.h
src/game/map/MapVision.cpp
src/game/map/MapVision.h
src/game/map/MapWalkability.cpp
src/game/map/MapWalkability.h
src/game/map/TileDistance.cpp
src/game/map/TileDistance.h
src/game/map/TileFields.cpp
src/game/map/TileFields.h
src/game/map/TileTriggers.cpp
src/game/map/TileTriggers.h
```

### layers

```text
src/layers/Layer.cpp
src/layers/Layer.h
src/layers/LayerManager.cpp
src/layers/LayerManager.h
src/layers/ui/LayerDropConfirm.cpp
src/layers/ui/LayerDropConfirm.h
src/layers/ui/LayerEquipRunes.cpp
src/layers/ui/LayerEquipRunes.h
src/layers/ui/LayerGiveContext.cpp
src/layers/ui/LayerGiveContext.h
src/layers/ui/LayerInventory.cpp
src/layers/ui/LayerInventory.h
src/layers/ui/LayerInventoryContext.cpp
src/layers/ui/LayerInventoryContext.h
src/layers/ui/LayerMagic.cpp
src/layers/ui/LayerMagic.h
src/layers/ui/LayerPickUp.cpp
src/layers/ui/LayerPickUp.h
src/layers/ui/LayerPickUpContext.cpp
src/layers/ui/LayerPickUpContext.h
src/layers/ui/LayerPopupText.cpp
src/layers/ui/LayerPopupText.h
src/layers/ui/LayerSpecialEvent.cpp
src/layers/ui/LayerSpecialEvent.h
src/layers/ui/LayerSpellCast.cpp
src/layers/ui/LayerSpellCast.h
src/layers/ui/LayerSpellInfo.cpp
src/layers/ui/LayerSpellInfo.h
src/layers/ui/LayerWorld.cpp
src/layers/ui/LayerWorld.h
```

### lib

```text
src/lib/Json.cpp
src/lib/Json.h
src/lib/StringUtil.h
src/lib/hiscore/hiscore.cpp
src/lib/hiscore/hiscore.h
```

### model

```text
src/model/Combat.cpp
src/model/Combat.h
src/model/instances/CharacterInstance.h
src/model/instances/CharacterPlayer.cpp
src/model/instances/CharacterPlayer.h
src/model/instances/ItemInstance.h
src/model/instances/MapInstance.cpp
src/model/instances/MapInstance.h
src/model/instances/Player.cpp
src/model/instances/Player.h
src/model/instances/TileInstance.h
src/model/instances/World.h
src/model/stats/CharacterDerivedStatDefinitions.cpp
src/model/stats/CharacterDerivedStatDefinitions.h
src/model/stats/CharacterDerivedStats.cpp
src/model/stats/CharacterDerivedStats.h
src/model/stats/CharacterStatDefinitions.cpp
src/model/stats/CharacterStatDefinitions.h
src/model/stats/CharacterStats.cpp
src/model/stats/CharacterStats.h
src/model/templates/Abilities.h
src/model/templates/AbilityTypes.cpp
src/model/templates/AbilityTypes.h
src/model/templates/CharacterTemplate.cpp
src/model/templates/CharacterTemplate.h
src/model/templates/Items.cpp
src/model/templates/Items.h
src/model/templates/MapGrids.h
src/model/templates/Maps.cpp
src/model/templates/Maps.h
src/model/templates/RuneTypes.cpp
src/model/templates/RuneTypes.h
src/model/templates/SpecialEvents.h
src/model/templates/Spells.h
src/model/templates/StatusEffects.h
src/model/templates/Tileset.h
src/model/templates/UtilityTypes.cpp
src/model/templates/UtilityTypes.h
```

### runner

```text
src/runner/ConditionEvaluator.cpp
src/runner/ConditionEvaluator.h
src/runner/EventRunnerHelpers.cpp
src/runner/EventRunnerHelpers.h
src/runner/SpecialEventRunner.cpp
src/runner/SpecialEventRunner.h
src/runner/StringEvaluator.cpp
src/runner/StringEvaluator.h
```

### state

```text
src/state/AbstractAction.h
src/state/ActionBus.cpp
src/state/ActionBus.h
src/state/DatabaseInterface.cpp
src/state/DatabaseInterface.h
src/state/LayerManagerInterface.cpp
src/state/LayerManagerInterface.h
src/state/State.h
src/state/StateManager.cpp
src/state/StateManager.h
src/state/StateManagerInterface.cpp
src/state/StateManagerInterface.h
src/state/Triggers.h
src/state/UiManager.cpp
src/state/UiManager.h
src/state/WorldActions.h
src/state/WorldUpdater.cpp
src/state/WorldUpdater.h
src/state/actions/combat/ActionBase.hpp
src/state/actions/combat/CharacterSetSpriteIndexOffset.hpp
src/state/actions/combat/DoCPUCombatTurn.hpp
src/state/actions/combat/DoCombatAction.hpp
src/state/actions/combat/DoCombatActionCompletion.hpp
src/state/actions/combat/EndCombat.hpp
src/state/actions/combat/GoNextCombatTurn.hpp
src/state/actions/combat/ModifyAP.hpp
src/state/actions/combat/ModifyHP.hpp
src/state/actions/combat/MoveCharacter.hpp
src/state/actions/combat/PerformCharacterDefeated.hpp
src/state/actions/combat/PerformMeleeAttack.hpp
src/state/actions/combat/PerformSpellCast.hpp
src/state/actions/combat/RemoveCharacterFromMap.hpp
src/state/actions/combat/SetActiveCombatCharacter.hpp
src/state/actions/combat/StartCombat.hpp
src/state/actions/general/PlaySound.hpp
src/state/actions/ui/UiAdjustEquippedRune.hpp
src/state/actions/ui/UiCancelEquipRunes.hpp
src/state/actions/ui/UiCommitEquipRunes.hpp
src/state/actions/ui/UiDropInventoryItem.hpp
src/state/actions/ui/UiGiveInventoryItem.hpp
src/state/actions/ui/UiPickUpItem.hpp
src/state/actions/ui/UiPushFloatingNotification.hpp
src/state/actions/ui/UiRemoveFloatingNotification.hpp
src/state/actions/ui/UiRemoveLayer.hpp
src/state/actions/ui/UiReorderInventoryItem.hpp
src/state/actions/ui/UiSelectSpellCast.hpp
src/state/actions/ui/UiSetCurrentPartyMember.hpp
src/state/actions/ui/UiSetCurrentPartyMemberInventory.hpp
src/state/actions/ui/UiSetCurrentPartyMemberMagic.hpp
src/state/actions/ui/UiSetSelectedPartyMemberId.hpp
src/state/actions/ui/UiSetSpellReady.hpp
src/state/actions/ui/UiShowLayerDropContext.hpp
src/state/actions/ui/UiShowLayerEquipRunes.hpp
src/state/actions/ui/UiShowLayerGiveContext.hpp
src/state/actions/ui/UiShowLayerInventory.hpp
src/state/actions/ui/UiShowLayerInventoryContext.hpp
src/state/actions/ui/UiShowLayerMagic.hpp
src/state/actions/ui/UiShowLayerPickUp.hpp
src/state/actions/ui/UiShowLayerPickupContext.hpp
src/state/actions/ui/UiShowLayerPopupText.hpp
src/state/actions/ui/UiShowLayerSpecialEvent.hpp
src/state/actions/ui/UiShowLayerSpellCast.hpp
src/state/actions/ui/UiShowLayerSpellInfo.hpp
src/state/actions/ui/UiToggleEquipInventoryItem.hpp
src/state/actions/ui/UiToggleManaSlotRune.hpp
src/state/actions/ui/heldMove/UiUpdateHeldMove.hpp
src/state/actions/world/PerformTownMeleeAttack.hpp
src/state/actions/world/TownEnemyAiAfterPlayerMove.hpp
src/state/actions/world/WorldExamineAt.hpp
src/state/actions/world/WorldInteractAt.hpp
src/state/actions/world/WorldLoadActiveMap.hpp
src/state/actions/world/WorldMoveActionAim.hpp
src/state/actions/world/WorldMovePlayer.hpp
src/state/actions/world/WorldSetActionAim.hpp
src/state/actions/world/WorldSetActionMode.hpp
src/state/actions/world/WorldSetCamera.hpp
src/state/actions/world/WorldSetCameraMode.hpp
src/state/actions/world/WorldSpawnDamageParticle.hpp
src/state/actions/world/WorldSpawnPlayer.hpp
src/state/actions/world/WorldSpawnPlayerAtMarker.hpp
src/state/actions/world/WorldSpawnPlayerAtXY.hpp
src/state/actions/world/WorldSpawnProjectile.hpp
src/state/actions/world/WorldTalkAt.hpp
src/state/actions/world/WorldTravel.hpp
```

### ui

```text
src/ui/FontScale.cpp
src/ui/FontScale.h
src/ui/KeyboardHeldScroll.cpp
src/ui/KeyboardHeldScroll.h
src/ui/SdlPixels.h
src/ui/TextStyle.h
src/ui/UiElement.cpp
src/ui/UiElement.h
src/ui/colors.h
src/ui/components/ChCompactInfo.cpp
src/ui/components/ChCompactInfo.h
src/ui/components/ConfirmModal.cpp
src/ui/components/ConfirmModal.h
src/ui/components/FloatingNotification.cpp
src/ui/components/FloatingNotification.h
src/ui/components/FloatingNotificationSection.cpp
src/ui/components/FloatingNotificationSection.h
src/ui/components/InGameTitleBar.cpp
src/ui/components/InGameTitleBar.h
src/ui/components/ItemInfo.cpp
src/ui/components/ItemInfo.h
src/ui/components/MapView.cpp
src/ui/components/MapView.h
src/ui/components/PartyMemberIconSelector.cpp
src/ui/components/PartyMemberIconSelector.h
src/ui/components/PartyMemberSwitcher.cpp
src/ui/components/PartyMemberSwitcher.h
src/ui/components/TiledOverlay.cpp
src/ui/components/TiledOverlay.h
src/ui/components/TouchMovePad.cpp
src/ui/components/TouchMovePad.h
src/ui/components/borders/BorderDropShadow.cpp
src/ui/components/borders/BorderDropShadow.h
src/ui/components/borders/BorderInGame.cpp
src/ui/components/borders/BorderInGame.h
src/ui/components/borders/BorderInGameNarrow.cpp
src/ui/components/borders/BorderInGameNarrow.h
src/ui/components/borders/BorderInGameWide.cpp
src/ui/components/borders/BorderInGameWide.h
src/ui/components/borders/BorderModalSmall.cpp
src/ui/components/borders/BorderModalSmall.h
src/ui/components/borders/BorderModalStandard.cpp
src/ui/components/borders/BorderModalStandard.h
src/ui/components/lists/ListChCompactInfoHorizontal.cpp
src/ui/components/lists/ListChCompactInfoHorizontal.h
src/ui/components/lists/ListChCompactInfoVertical.cpp
src/ui/components/lists/ListChCompactInfoVertical.h
src/ui/components/lists/ListInventory.cpp
src/ui/components/lists/ListInventory.h
src/ui/components/lists/ListMagicSpells.cpp
src/ui/components/lists/ListMagicSpells.h
src/ui/components/lists/ListPickUp.cpp
src/ui/components/lists/ListPickUp.h
src/ui/elements/HorizontalList.cpp
src/ui/elements/HorizontalList.h
src/ui/elements/HorizontalSlider.cpp
src/ui/elements/HorizontalSlider.h
src/ui/elements/OutsetRectangle.cpp
src/ui/elements/OutsetRectangle.h
src/ui/elements/Quad.cpp
src/ui/elements/Quad.h
src/ui/elements/SectionScrollable.cpp
src/ui/elements/SectionScrollable.h
src/ui/elements/SpriteElement.cpp
src/ui/elements/SpriteElement.h
src/ui/elements/TextBanner.cpp
src/ui/elements/TextBanner.h
src/ui/elements/TextLine.cpp
src/ui/elements/TextLine.h
src/ui/elements/TextParagraph.cpp
src/ui/elements/TextParagraph.h
src/ui/elements/VerticalList.cpp
src/ui/elements/VerticalList.h
src/ui/elements/buttons/ButtonClose.cpp
src/ui/elements/buttons/ButtonClose.h
src/ui/elements/buttons/ButtonGroup.cpp
src/ui/elements/buttons/ButtonGroup.h
src/ui/elements/buttons/ButtonIcon.cpp
src/ui/elements/buttons/ButtonIcon.h
src/ui/elements/buttons/ButtonList.cpp
src/ui/elements/buttons/ButtonList.h
src/ui/elements/buttons/ButtonModal.cpp
src/ui/elements/buttons/ButtonModal.h
src/ui/elements/buttons/ButtonMove.cpp
src/ui/elements/buttons/ButtonMove.h
src/ui/elements/buttons/ButtonScroll.cpp
src/ui/elements/buttons/ButtonScroll.h
src/ui/elements/buttons/ButtonSprite.cpp
src/ui/elements/buttons/ButtonSprite.h
src/ui/elements/buttons/ButtonTextWrap.cpp
src/ui/elements/buttons/ButtonTextWrap.h
src/ui/elements/buttons/ButtonWorldAction.cpp
src/ui/elements/buttons/ButtonWorldAction.h
src/ui/helpers/keyboardShortcuts.cpp
src/ui/helpers/keyboardShortcuts.h
src/ui/helpers/modalLayoutFit.cpp
src/ui/helpers/modalLayoutFit.h
src/ui/helpers/worldActions.cpp
src/ui/helpers/worldActions.h
src/ui/layouts/InGameLayout.cpp
src/ui/layouts/InGameLayout.h
src/ui/layouts/ModalSmall.cpp
src/ui/layouts/ModalSmall.h
src/ui/layouts/ModalStandard.cpp
src/ui/layouts/ModalStandard.h
src/ui/minipages/MinipageCharacterSheet.cpp
src/ui/minipages/MinipageCharacterSheet.h
src/ui/minipages/MinipageEquipRunes.cpp
src/ui/minipages/MinipageEquipRunes.h
src/ui/minipages/MinipageEvent.cpp
src/ui/minipages/MinipageEvent.h
src/ui/minipages/MinipagePickUp.cpp
src/ui/minipages/MinipagePickUp.h
src/ui/minipages/MinipageSpellCast.cpp
src/ui/minipages/MinipageSpellCast.h
src/ui/observers/ObserverAdjustEquippedRune.hpp
src/ui/observers/ObserverCancelEquipRunes.hpp
src/ui/observers/ObserverCancelWorldActionMode.hpp
src/ui/observers/ObserverCommitEquipRunes.hpp
src/ui/observers/ObserverDropInventoryItem.hpp
src/ui/observers/ObserverGiveInventoryItem.hpp
src/ui/observers/ObserverInventorySelectItem.hpp
src/ui/observers/ObserverPickUpItem.hpp
src/ui/observers/ObserverRemoveLayer.hpp
src/ui/observers/ObserverReorderInventoryItem.hpp
src/ui/observers/ObserverSelectSpellCast.hpp
src/ui/observers/ObserverSetCurrentPartyMember.hpp
src/ui/observers/ObserverSetCurrentPartyMemberInventory.hpp
src/ui/observers/ObserverSetCurrentPartyMemberMagic.hpp
src/ui/observers/ObserverSetSelectedPartyMemberId.hpp
src/ui/observers/ObserverSetSpellReady.hpp
src/ui/observers/ObserverShowLayerDropContext.hpp
src/ui/observers/ObserverShowLayerEquipRunes.hpp
src/ui/observers/ObserverShowLayerGiveContext.hpp
src/ui/observers/ObserverShowLayerInventoryContext.hpp
src/ui/observers/ObserverShowLayerPickUpContext.hpp
src/ui/observers/ObserverShowLayerPopupText.hpp
src/ui/observers/ObserverShowLayerSpellInfo.hpp
src/ui/observers/ObserverSpecialEventChoice.hpp
src/ui/observers/ObserverSpecialEventContinue.hpp
src/ui/observers/ObserverToggleManaSlotRune.hpp
src/ui/observers/ObserverUpdateCurrentPartyMember.hpp
src/ui/observers/ObserverWorldAction.hpp
src/ui/pages/PageCharacter.cpp
src/ui/pages/PageCharacter.h
src/ui/pages/PageInventory.cpp
src/ui/pages/PageInventory.h
src/ui/pages/PageMagicSetup.cpp
src/ui/pages/PageMagicSetup.h
src/ui/pages/PageModalEvent.cpp
src/ui/pages/PageModalEvent.h
src/ui/pages/PageTalkChoice.cpp
src/ui/pages/PageTalkChoice.h
src/ui/popups/PopupDropConfirm.cpp
src/ui/popups/PopupDropConfirm.h
src/ui/popups/PopupGive.cpp
src/ui/popups/PopupGive.h
src/ui/popups/PopupInventoryItem.cpp
src/ui/popups/PopupInventoryItem.h
src/ui/popups/PopupPickupItem.cpp
src/ui/popups/PopupPickupItem.h
src/ui/popups/PopupSpellInfo.cpp
src/ui/popups/PopupSpellInfo.h
src/ui/uiUtils.h
```

## Phase verification ledger

| Phase | Commit | Verification |
|---|---|---|
| 0 | `Record the header restoration baseline` (this manifest's commit) | PASS: fixed references and 20 interfaces inventoried; baseline commands recorded; tracked diff contains this manifest only |
| 1 | `Add the conventional pinned-dependency CMake build` (pending commit hash) | PASS with user-approved platform deferral: local GCC/Clang debug/release, Make parity, dependency diagnostics, no-op build, and representative wrappers pass; UCRT64 is deferred to final qualification and Emscripten remains part of the final supported-host matrix |
| 2 | `50a4885` (`Restore CMake-backed test runners`); `Retire the transitional Make build` (pending follow-up hash) | PASS locally: 81 distinct CMake test targets; all 38 non-UI tests pass under GCC and Clang; all 79 legacy wrappers pass from `/private/tmp` (43 UI wrappers with `--build-only`); aggregate UI compilation passes under GCC and Clang; an induced compile error propagates exit status 1; user approved deferring MSYS2 qualification until the final gate |
| 3 | pending | pending |
| 4 | pending | pending |
| 5 | pending | pending |
| 6 | pending | pending |
| 7 | pending | pending |
| 8 | pending | pending |

## Known stale, disabled, or retired tests

No test is approved for retirement or disablement. Add a row here before changing any test's retained status.

| Test | Classification | Evidence / approval |
|---|---|---|
| Representative baseline wrappers | resolved in Phases 1–2 | Pinned header compatibility fixes and CMake-backed wrapper targets compile and run successfully |
| Previously compile-disabled spell/enemy tests | resolved in Phase 2 | Restored the authored spell-rule implementation and updated enemy orchestration to enqueue the explicit action; focused tests pass under GCC and Clang |
| Previously runtime-disabled asset/coordinate tests | resolved in Phase 2 | Expectations now match current spell/map-grid assets and world-grid coordinates; all five tests pass under GCC and Clang |
