# Development

Carcer is built as a conventional C++23 header/source project with CMake 3.28+,
Ninja, Make, and clangd. Native builds require GCC or Clang plus SDL2,
SDL2_image, SDL2_ttf, SDL2_mixer, and SDL2_gfx. Web builds require an activated
Emscripten SDK.

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

The pinned upstream revisions retain both APIs, but Carcer stages only their
classic headers and static archives under ignored `build/compat/`, keyed by
compiler, target, flags, and lock identity. This keeps dependency work reusable
without adding an alternate build graph to the game.

## Native builds

```sh
./scripts/setup-dev.sh
cd src
make
make run
make test
```

The setup script checks the native tools and SDL packages, materializes the
pinned dependencies, configures the current-platform `dev-debug` preset,
generates clangd's compilation database, and builds the game. Set `CXX` before
the first setup to choose a compiler. The `src/Makefile` is only a convenience
front end; CMake remains the one authoritative build graph.

Explicit `gcc-debug`, `gcc-release`, `clang-debug`, and `clang-release` presets
remain available for qualification. Compiler selection is an ordinary CMake
cache choice, so a local override also works:

```sh
cmake --preset clang-debug -DCMAKE_CXX_COMPILER=/path/to/clang++
```

The first build prepares the pinned header dependency bundle. A second build is
a no-op apart from fast pinned-revision validation. Production sources use one
CMake-managed precompiled header and an object library; application-only builds
do not duplicate those objects in a static archive. Test targets create the
archive only when needed for selective test linking.

### Header naming convention

- `Name.h` is the public interface for a sibling `Name.cpp`.
- `Name.hpp` is header-only and must not have a sibling implementation file.
- Production `.cpp` files require a same-directory `.h`, except for
  `src/main.cpp`. Standalone test programs under `src/__test__/` are also
  exempt.

When adding a production implementation, create its `.h`/`.cpp` pair and
register the `.cpp` in the appropriate domain-grouped list in
`cmake/carcer_sources.cmake`. Header-only additions need only the `.hpp`
file.

## Windows with MSYS2 UCRT64

Open a UCRT64 terminal and install the packages in the README. That shell
already has the toolchain on `PATH`:

```sh
./scripts/setup-dev.sh
make -C src
make -C src test
```

`ucrt64-release` is the release equivalent. The Windows executable bypasses
SDL2main consistently and calls `SDL_SetMainReady` through
`cmake/win_sdl_main_ready.cpp`.

PowerShell does not start inside that environment. `scripts/Invoke-Ucrt64.ps1`
opens a UCRT64 shell for callers that need one. Agent instructions use that
wrapper; a UCRT64 terminal does not.

## Emscripten

Activate the SDK so `EMSDK` is set, then run:

```sh
cmake --preset emscripten-debug
cmake --build --preset emscripten-debug --target CARCER
```

`emscripten-release` is the release equivalent. The target retains the SDL
ports, asset preload, memory settings, exported functions, and JavaScript
runtime methods required by the web build. After setup, `make js` from `src/`
builds the release and copies `CARCER.js`, `.wasm`, and `.data` into `web/`.

## Tests

Every test source has a distinct CMake executable. Non-UI tests run through
CTest; UI tests are compile-only by default so automation never opens SDL
windows:

```sh
cmake --build --preset gcc-debug --target carcer_non_ui_tests
ctest --preset gcc-debug
cmake --build --preset gcc-debug --target carcer_ui_tests
cmake --build --preset gcc-debug --target carcer_header_checks
```

CTest also runs the complete public-header self-containment target. The
documented production direction is
`model -> rules -> state -> actions -> ui -> layers -> app`; higher domains may
depend on lower ones.

The scripts under `test-runners/` preserve their previous interface and may be
called from any directory. Set `CARCER_CMAKE_PRESET` to select another build.
Non-UI wrappers run by default and honor `--build-only`; UI wrappers pass
remaining arguments to their executable when run interactively.

## IDE setup

CMake writes `build/cmake/dev-debug/compile_commands.json` during setup.
`scripts/generate-clangd-config.sh` (also run by `setup-dev.sh`) writes a
machine-local `.clangd` that points at that database and, on MSYS2 UCRT64,
wires Clang's resource directory plus libstdc++ from your current
`MINGW_PREFIX` (so installs under `C:\msys64`, Scoop, etc. all work without
hardcoded paths). `.clangd` is gitignored; re-run the generator after moving
MSYS2 or changing the toolchain.

On Windows, `generate-clangd-config.sh` writes `clangd.path` in
`.vscode/settings.json` to that install's `ucrt64/bin/clangd.exe`. The editor
has to spawn that executable directly. A `.cmd` or other wrapper breaks the
language-server pipe, so go-to-definition never comes up. Linux and macOS
checkouts should clear `clangd.path` so the editor uses `clangd` from `PATH`.

Install `mingw-w64-ucrt-x86_64-clang` and `mingw-w64-ucrt-x86_64-clang-tools-extra`
in the UCRT64 environment. VS Code and Cursor recommend clangd and CMake Tools
on first open; Microsoft C/C++ IntelliSense is disabled to prevent duplicate
diagnostics while its debugger remains available.

## Localization and animation tools

`scripts/update-translations.sh` and `scripts/anims.sh` validate and use the
pinned SDL2W/BMIN checkouts under `.deps/`; they no longer rely on mutable
sibling repositories.
