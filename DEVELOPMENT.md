# Development

Carcer is built as a C++23 named-module project with CMake 3.28+ and Ninja.
Native builds require GCC 14+ or Clang 16+ with `clang-scan-deps`, plus SDL2,
SDL2_image, SDL2_ttf, SDL2_mixer, and SDL2_gfx. Web builds require an activated
Emscripten SDK. See [src/modules/MODULES.md](src/modules/MODULES.md) for the
module boundaries and dependency rules.

## Dependencies

SDL2W and BMIN revisions are pinned in `deps.lock` and materialized under the
ignored `.deps/` directory. Normal configure and build commands validate these
checkouts but never clone, fetch, switch, or modify them.

```sh
./scripts/bootstrap-deps.sh
./scripts/bootstrap-deps.sh --check
```

Use `--repair` to fetch and select the locked revisions. For local mirrors, set
`CARCER_SDL2W_REPOSITORY` and `CARCER_BMIN_REPOSITORY`. Tracked dependency edits
are rejected unless `CARCER_ALLOW_DIRTY_DEPS=1` is explicitly set.

## Native build and tests

```sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
```

Equivalent `gcc-release`, `clang-debug`, and `clang-release` configure, build,
and test presets live in `CMakePresets.json`. GCC presets resolve `g++-15` from
`PATH`. Clang presets search common Homebrew LLVM paths and require
`clang-scan-deps`; override `CMAKE_CXX_COMPILER` for another installation.

The default native build includes the game, enabled runtime tests, and module
import/architecture tests. Five pre-existing runtime tests are explicitly
disabled because their expectations are stale; three more sources are excluded
because they call disabled production APIs. Both lists are in `CMakeLists.txt`.

Compile all 43 interactive UI tests without running them:

```sh
cmake --build --preset gcc-debug --target carcer_ui_tests
# or
CARCER_CMAKE_PRESET=gcc-debug ./scripts/compile-ui-tests.sh
```

Build or run one test through its existing convenience wrapper:

```sh
bash test-runners/runner/TestJson.sh
bash test-runners/ui/TestConfirmModal.sh --build-only
```

The wrappers use `gcc-debug` by default. Set `CARCER_CMAKE_PRESET` to select a
different configured toolchain. UI executables open SDL windows and must not be
run by unattended automation.

## Windows with MSYS2 UCRT64

Install CMake, Ninja, a GCC 14+ UCRT64 toolchain, and the UCRT64 SDL packages.
Run commands from PowerShell through the repository wrapper:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --preset ucrt64-debug"
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset ucrt64-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset ucrt64-debug"
.\scripts\Invoke-Ucrt64.ps1 "./scripts/compile-ui-tests.sh"
```

`Invoke-Ucrt64.ps1` discovers common MSYS2 locations. Set `MSYS2_ROOT` when the
installation is elsewhere. The `ucrt64-release` preset is also available.

## Emscripten and web distribution

Activate the SDK so `EMSDK` is set, then run:

```sh
cmake --preset emscripten-debug
cmake --build --preset emscripten-debug
```

`emscripten-release` is the release equivalent. `npm run build` uses that
preset, copies `CARCER.js`, `CARCER.wasm`, and `CARCER.data` into `web/`, then
assembles `dist/`.

## IDE setup

CMake exports `compile_commands.json` into each preset build directory. The
checked-in VS Code settings point clangd at `build/cmake/clang-debug`; configure
that preset before indexing:

```sh
cmake --preset clang-debug
```

Keep `--experimental-modules-support` in clangd's arguments, not `.clangd`
compile flags.

## Qualification commands

```sh
./scripts/benchmark-fresh-modules.sh gcc-debug 3 8
./scripts/validate-native-repeatability.sh gcc-debug 10 8
./scripts/validate-dependency-headers.sh g++-15
./scripts/modules/check_ui_architecture.sh
```

Benchmark output is written below ignored `build/benchmarks/` paths. The
architecture check is also registered with CTest.

## Localization and animation tools

`scripts/update-translations.sh` builds SDL2W's pinned `L10nScanner`, scans
`src/`, and updates the translation files. New player-visible text must use
`TRANSLATE("...")`; developer logs and diagnostics remain untranslated.

`scripts/anims.sh` similarly builds and invokes SDL2W tooling from `.deps/`.
These scripts use the dependencies' own build interfaces; they do not build
Carcer.
