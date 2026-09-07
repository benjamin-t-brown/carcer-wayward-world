# C++ Module Experiment Retrospective

Date: 2026-09-07

## Decision

Carcer is returning to a conventional C++23 header/source architecture. The
module experiment produced real architectural improvements, but its final form
did not make the game easier or faster enough to justify its build and editing
costs. The preserved `experiment/cpp-modules` branch remains the historical
record.

SDL2W and BMIN stay pinned to the approved upstream revisions that support both
headers and named modules. Carcer deliberately consumes only their header API.
This lets the dependency projects continue toward their own dual-capable
release without making Carcer responsible for BMI ordering or compiler-specific
module artifacts.

## What the experiment proved

The strongest win was dependency direction. The work exposed invalid upward
dependencies and led to an enforceable domain flow:

```text
model -> rules -> state -> actions -> ui -> layers -> app
```

The arrow means “may be consumed by.” Model remains data-oriented; rules are
independent of state and presentation; actions express navigation through
neutral state requests; UI does not own loop segments; layers sit above UI and
own activation, event, update, and rendering policy; the application is the
composition root.

Other improvements retained from the experiment include:

- pinned, validated, repository-local SDL2W/BMIN bootstrapping;
- compiler/configuration-keyed classic-header dependency bundles;
- CMake/Ninja presets for GCC, Clang, UCRT64, and Emscripten;
- stable semantic action events instead of RTTI/downcasts;
- explicit map, combat, persistence, and state ownership;
- `Layer` as a non-visual loop concept, with `UiLayer` as an optional visual
  specialization;
- the SDL2W behavior upgrades for cache bounds, lazy fonts, asset validation,
  deterministic window lifecycle, double delta time, opt-in controllers,
  events, and audio;
- automated include-direction and public-header self-containment checks.

## Measured result of modules

The module branch greatly reduced physical source count. At its final commit,
the production tree had 92 C++ paths (71 `.cpp`, 20 `.cppm`, and one retained
header), compared with 384 paths on the pre-module header reference (136 `.cpp`
and 248 headers). The final module graph had 20 interfaces, 70 import edges, a
critical depth of 10, and maximum fan-out of 16.

That reduction did not translate into a better cold build. The first module
design measured 318.28 seconds with 257 compiled project units. After compiler
scanning, coarser interfaces, grouped implementations, and UI graph cleanup,
the final GCC debug application builds measured 68, 69, and 69 seconds (69
second median). A bounded compiler-flag follow-up reached a 68 second median,
within normal variance, and was reverted because it reduced debugger fidelity.

Final module measurements were:

| Metric | Module result |
|---|---:|
| Fresh GCC debug application build | 68, 69, 69 s; median 69 s |
| No-op build | 1, 0, 0 s; median 0 s |
| Leaf implementation rebuild | 3, 3, 3 s |
| Objects, archives, and BMIs | 277,056 KiB |
| Dependency-scan preprocessing data | median 204,932 KiB |
| Entire build directory | median 488,816 KiB |

Ninja timing attributed roughly 82 aggregate seconds to dependency scanning
and 267 aggregate seconds to compilation. Module order was correct and stable,
but the required scan/BMI graph remained substantial work and constrained the
build's critical path.

## Why the source-count win was not enough

The smallest module file count was achieved partly by combining many unrelated
or independently edited classes. Actions fell from 77 files to three, while 102
UI/layer files fell to 20. That made broad boundaries visible, but it also made
routine navigation, review, merge conflict isolation, and class-level reasoning
worse. A lower path count is not valuable when each path becomes a large catalog
of unrelated declarations or implementations.

Bootstrap complexity also moved in the wrong direction. A correct build needed
compiler-aware scanning, ordered BMI production, cache identity management,
special handling for GCC and Clang, and substantial retained preprocessing
output. Those mechanisms solved real compiler constraints but were complexity
created by the representation rather than by the game.

The experiment therefore failed its original combined objective: fewer files,
better compile time, and cleaner architecture. It achieved the first goal and
helped discover the third, but did not achieve the second, and its file
consolidation harmed editability.

## Restored header result

The restored production tree contains 408 C++ paths: 144 `.cpp` implementation
units and 264 class- or concern-level headers. A same-method recount gives
36,561 production C++ lines at the old header reference, 36,677 on the final
module branch, and 37,192 in the restored tree. The increased file count is
intentional: actions and UI/layer classes again have readable, local homes.

One conventional CMake-managed precompiled header is used for common BMIN and
SDL2W headers. Before that bounded optimization, fresh GCC debug builds measured
42.06, 42.09, and 42.15 seconds. With it, final measurements are:

| Metric | Old header baseline | Final restored headers | Final modules |
|---|---:|---:|---:|
| Fresh GCC debug application build | 34.22 s | 32.17, 32.27, 33.91 s; median 32.27 s | median 69 s |
| No-op build | not recorded | 0.18, 0.18, 0.20 s; median 0.18 s | median 0 s |
| Leaf implementation rebuild | 3.03 s | 2.69 s | median 3 s |
| Build artifacts | about 217 MiB | 373,292 KiB including the PCH | 277,056 KiB objects/archives/BMIs |
| Entire measured build directory | not recorded | 379,712 KiB | median 488,816 KiB |

The PCH trades approximately 96 MiB of explicit compiler cache for a roughly
ten-second cold-build reduction. It is a single portable CMake feature, has no
hand-maintained dependency order, and keeps the total build directory smaller
than the module result because there is no retained scan-preprocessing tree.
The leaf test rebuilt only `FontScale.cpp`, the static archive, and the
application link.

## Why CMake remains

CMake is retained. It adds a project description layer, but unlike the custom
module machinery it provides direct value independent of the language
architecture: compiler/toolchain selection, isolated debug/release directories,
UCRT64 and Emscripten presets, generated compile commands, CTest registration,
compile-only UI targets, and imported pinned dependency archives. The former
Make graph was removed so CMake is the one authoritative Carcer build graph.

The boundary is deliberate: CMake describes ordinary translation units and one
ordinary PCH; `scripts/bootstrap-deps.sh` performs explicit network-changing
dependency operations; configure/build only validate pinned checkouts and stage
classic headers/archives. There are no Carcer module interfaces, named imports,
BMI manifests, module probes, or generated module-order files.

## Final qualification

GCC 15 and Clang 22 debug and release builds pass on the qualification host.
Each configuration passes all 41 final CTest entries: 39 behavioral tests, the
complete include-direction check, and compilation of all 264 production headers
in isolation. Both debug configurations also compile every UI executable. All
79 legacy shell runners work from outside the repository (36 behavioral runs
and 43 UI build-only runs), and the pinned dependency validation and forced
classic-header dependency rebuild pass.

The cross-platform presets and entry points remain checked in, but this macOS
host cannot honestly qualify their toolchains. Emscripten is unavailable because
no EMSDK is installed or activated. MSYS2 UCRT64 and PowerShell are unavailable;
the three UCRT64 shell entry points pass Bash syntax validation. Debug and
release execution for both targets therefore remains a supported-host follow-up,
not a claimed local pass.

## Going forward

Use modules only where their ownership and toolchain cost can be evaluated in a
smaller independent library. For Carcer, preserve the architectural lessons
with tests and include rules, not with a language mechanism. Optimize build
cost with conventional, measured techniques while keeping class-level files and
clear ownership as non-negotiable maintainability constraints.
