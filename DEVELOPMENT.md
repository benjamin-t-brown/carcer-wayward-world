# Development

Carcer is built as a conventional C++23 header/source project with CMake 3.28+
and Ninja. Native builds require GCC or Clang plus SDL2, SDL2_image, SDL2_ttf,
SDL2_mixer, and SDL2_gfx. Web builds require an activated Emscripten SDK.

## Pinned dependencies

SDL2W and BMIN are pinned in `deps.lock` and materialized under ignored
`.deps/` checkouts:

```sh
./scripts/bootstrap-deps.sh
./scripts/bootstrap-deps.sh --check
```

Configure and build only validate existing checkouts; they never clone, fetch,
switch, or modify them. Use `--repair` to fetch and select locked revisions.
For offline mirrors, set `CARCER_SDL2W_REPOSITORY` and
`CARCER_BMIN_REPOSITORY`. Tracked dependency changes are rejected unless
`CARCER_ALLOW_DIRTY_DEPS=1` is explicitly selected.

The pinned upstreams can build both modules and headers. Carcer stages only
their header archives and include trees under ignored `build/compat/`, keyed by
compiler, target, flags, and lock identity. This keeps the dependency work
reusable without adding module scanning or BMI ordering to the game.

## Native builds

```sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug --target CARCER
```

Equivalent `gcc-release`, `clang-debug`, and `clang-release` presets are in
`CMakePresets.json`. Compiler selection is an ordinary CMake cache choice, so
a local override also works:

```sh
cmake --preset clang-debug -DCMAKE_CXX_COMPILER=/path/to/clang++
```

The first build prepares the pinned header dependency bundle. A second build is
a no-op apart from fast pinned-revision validation. `src/Makefile` remains
temporarily available as a parity oracle until the test-runner migration in
Phase 2; it is not the long-term build interface.

## Windows with MSYS2 UCRT64

Install CMake, Ninja, a UCRT64 GCC toolchain, and the UCRT64 SDL packages. From
PowerShell:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "./scripts/bootstrap-deps.sh"
.\scripts\Invoke-Ucrt64.ps1 "cmake --preset ucrt64-debug"
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset ucrt64-debug --target CARCER"
```

`ucrt64-release` is the release equivalent. `Invoke-Ucrt64.ps1` discovers
common MSYS2 installations; set `MSYS2_ROOT` when yours is elsewhere. The
Windows executable bypasses SDL2main consistently and calls `SDL_SetMainReady`
through `cmake/win_sdl_main_ready.cpp`.

## Emscripten

Activate the SDK so `EMSDK` is set, then run:

```sh
cmake --preset emscripten-debug
cmake --build --preset emscripten-debug --target CARCER
```

`emscripten-release` is the release equivalent. The target retains the SDL
ports, asset preload, memory settings, exported functions, and JavaScript
runtime methods required by the web build.

## Dependency module compatibility

Carcer does not compile named modules. When changing SDL2W or BMIN themselves,
their module-capable upstream branches remain testable independently:

```sh
make -C .deps/bmin/src/modules check
make -C .deps/sdl2w/src/modules check BMIN_REPO="$PWD/.deps/bmin"
```

Those commands are opt-in and never add module flags to a Carcer target.

## IDE setup

CMake writes `compile_commands.json` into each preset build directory. Point
clangd at the configured directory, for example `build/cmake/clang-debug`. No
module-specific clangd option is required.

## Localization and animation tools

`scripts/update-translations.sh` and `scripts/anims.sh` validate and use the
pinned SDL2W/BMIN checkouts under `.deps/`; they no longer rely on mutable
sibling repositories.
