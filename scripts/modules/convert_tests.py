#!/usr/bin/env python3
"""Strip classic game/bmin/sdl2w includes from __test__ sources; keep imports."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TEST = ROOT / "src" / "__test__"

INCLUDE_RE = re.compile(
    r'^\s*#include\s*([<"][^>"]+[>"])\s*(?://.*)?\s*$'
)

DOMAIN = {
    "bmin",
    "sdl2w",
    "db",
    "game",
    "layers",
    "lib",
    "model",
    "runner",
    "state",
    "ui",
}


def should_strip(inc: str) -> bool:
    if inc.startswith("<"):
        return False
    path = inc.strip('"').replace("\\", "/")
    if path == "macros.h" or path.endswith("/macros.h"):
        return False
    if path.endswith("setupTestUi.h"):
        return False
    parts = path.split("/")
    return any(p in DOMAIN for p in parts)


def convert_cpp(text: str) -> str:
    lines = text.splitlines()
    setup_incs: list[str] = []
    std_incs: list[str] = []
    body_lines: list[str] = []
    in_preamble = True

    for line in lines:
        if in_preamble:
            if line.strip() in ("", "import carcer;", "import sdl2w;") or line.strip().startswith(
                "import "
            ):
                continue
            m = INCLUDE_RE.match(line)
            if m:
                inc = m.group(1)
                raw = inc.strip('<>"')
                if raw.endswith("setupTestUi.h"):
                    setup_incs.append(f"#include {inc}")
                elif inc.startswith("<"):
                    if inc.strip("<>") == "SDL.h":
                        continue
                    std_incs.append(f"#include {inc}")
                elif should_strip(inc):
                    continue
                elif raw == "macros.h" or raw.endswith("/macros.h"):
                    continue
                else:
                    std_incs.append(f"#include {inc}")
                continue
            # first real body line
            in_preamble = False
            body_lines.append(line)
        else:
            # also strip leftover classic includes mid-file
            m = INCLUDE_RE.match(line)
            if m and should_strip(m.group(1)):
                continue
            body_lines.append(line)

    needed = [
        "#include <functional>",
        "#include <ctime>",
        "#include <cstdlib>",
        "#include <memory>",
        "#include <string_view>",
    ]
    seen: set[str] = set()
    std_ordered: list[str] = []
    for s in needed + std_incs:
        if s not in seen:
            seen.add(s)
            std_ordered.append(s)
    out = std_ordered + [
        "import carcer;",
        "import sdl2w;",
        "import bmin.string_interop;",
        '#include "macros.h"',
    ]
    out.extend(setup_incs)
    out.append("")
    out.extend(body_lines)
    return "\n".join(out).replace("\r\n", "\n") + "\n"


def main() -> None:
    setup = TEST / "setupTestUi.h"
    setup.write_text(
        """#pragma once

// Requires the including TU to have:
//   import carcer;
//   import sdl2w;
//   #include "macros.h"

#include <functional>

struct TestUiParams {
  int width;
  int height;
  bmin::String title = "UI Test";
};

// Runs after the render loop exits, while window/store are still alive.
// Clear UI elements here so Quad and other SDL-owned resources are released
// before the window is destroyed and SDL_Quit is called.
inline void setupTestUi(int argc,
                        char** argv,
                        const TestUiParams& params,
                        std::function<void(sdl2w::Window&, sdl2w::Store&)> _init,
                        std::function<bool(sdl2w::Window&, sdl2w::Store&)> _updateRender,
                        std::function<void()> _teardown = {}) {
  LOG(INFO) << "Starting UI Test: " << params.title << LOG_ENDL;
  sdl2w::Window::init();

  {
    sdl2w::Store store;
    sdl2w::Window window(store,
                         {
                             .mode = sdl2w::DrawMode::GPU,
                             .title = params.title.cStr(),
                             .w = params.width,
                             .h = params.height,
                             .x = 25,
                             .y = 50,
                             .renderW = params.width,
                             .renderH = params.height,
                         });

    sdl2w::L10n::init({{"en"}});
    sdl2w::setupStartupArgs(argc, argv, window);
    sdl2w::L10n::setLanguage(DISABLE_TRANSLATIONS);
    window.getDraw().setBackgroundColor({0, 0, 0});

    sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
    window.getStore().loadAndStoreFont("title", "assets/squealer.ttf");
    window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");
    window.getStore().loadAndStoreFont("alternate", "assets/monofonto.ttf");
    window.getStore().loadAndStoreFont("text", "assets/notosans-regular.ttf");
    window.getStore().loadAndStoreFont("text-bold", "assets/notosans-bold.ttf");
    assetLoader.loadAssetsFromFile(sdl2w::ASSET_FILE, "assets/assets.ui.txt");
    assetLoader.loadAssetsFromFile(sdl2w::ASSET_FILE, "assets/assets.game.txt");

    window.setSoundPct(33);

    auto _initializeLoop = [&]() {
      sdl2w::renderSplash(window);
      return true;
    };

    auto _onInitialized = [&]() {
      LOG(INFO) << "Initializing test..." << LOG_ENDL;
      _init(window, store);
    };

    auto _mainLoop = [&]() {
      window.getDraw().setBackgroundColor({60, 60, 60});

#ifndef __EMSCRIPTEN__
      if (window.getEvents().isKeyPressed("q") ||
          window.getEvents().isKeyPressed("Q")) {
        return false;
      }
#endif

      return _updateRender(window, store);
    };

    window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);

    if (_teardown) {
      _teardown();
    }
  }

  sdl2w::Window::unInit();
}
""",
        encoding="utf-8",
        newline="\n",
    )
    print("rewrote setupTestUi.h")

    for p in sorted(TEST.rglob("*.cpp")):
        old = p.read_text(encoding="utf-8", errors="replace")
        new = convert_cpp(old)
        p.write_text(new, encoding="utf-8", newline="\n")
        print("converted", p.relative_to(ROOT))


if __name__ == "__main__":
    main()
