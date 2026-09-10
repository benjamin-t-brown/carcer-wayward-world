# Phase 4 browser verification

This protocol verifies the map kernel against real game data without writing to
the live `src/assets/db` directory. It complements the deterministic Node
renderer benchmark and tests; it does not replace visual inspection or a real
browser performance trace.

## Current evidence

Automated coverage currently establishes:

- DPR 1, DPR 2, fractional-DPR backing sizes, logical pointer conversion, and
  pointer-anchored zoom in `test/maps/canvas-metrics.test.ts`.
- Visible-region culling, logical-size culling with a DPR-sized backing store,
  independently positioned map blocks, opacity restoration, and stable draw
  counts in the renderer tests.
- Real Alinea topology: a focused-map-sized viewport plus one partition of
  overscan produces nine editable blocks around `alinea_outsideAlinea1`.
- A 1,000×1,000 synthetic grid enumerates only the nine visible/overscan cells
  for the tested viewport.
- Focused and neighboring hit testing, incompatible-neighbor protection, and
  map-local tileset-index translation in `test/maps/map-scene.test.ts`.
- One cross-grid gesture can modify two documents, remain one undo command,
  translate the selected tileset from local index 1 to local index 2, save only
  `maps.json`, and reload both changes in
  `test/integration/map-save-all.test.ts`.
- Generated small, medium, and large renderer scenarios have deterministic
  culling counts and command hashes. Local wall-clock reference numbers are in
  [map-performance-baseline.md](map-performance-baseline.md).

Run that evidence with:

```sh
node --import tsx --test \
  test/maps/canvas-metrics.test.ts \
  test/maps/map-renderer-blocks.test.ts \
  test/maps/map-scene.test.ts \
  test/maps/render-benchmark.test.ts \
  test/integration/map-save-all.test.ts
npm run benchmark:maps -- --warmup=200 --frames=2000 --json
```

These checks do not prove sprite correctness, seam-free visual composition, or
browser/GPU frame cost. Those remain browser checks below.

## Run against an isolated database

From `ceditor2`, create a temporary database copy:

```sh
CED2_VERIFY_ROOT="$(mktemp -d /tmp/ceditor2-phase4.XXXXXX)"
cp -R ../src/assets/db "$CED2_VERIFY_ROOT/db"
find "$CED2_VERIFY_ROOT/db" -type f -name '*.json' -exec shasum -a 256 {} \; \
  | sort > "$CED2_VERIFY_ROOT/before.sha256"
```

In a first terminal, run the API against that copy. The final argument is the
temporary database path; the real asset tree is still used read-only for sprite
images and SDL2W definitions.

```sh
node --import tsx --eval \
  "import { createApp } from './src/server/app.ts'; const app = createApp({ databasePath: process.argv[1] }); app.listen(3001, '127.0.0.1', () => console.log('isolated API ready'));" \
  "$CED2_VERIFY_ROOT/db"
```

In a second terminal:

```sh
npm run dev:client
```

Open this real grid map:

```text
http://127.0.0.1:3000/pages/maps/?map=alinea_outsideAlinea1
```

Do not use `npm run dev` for destructive verification: its default server owns
the live database.

After the run, prove the write boundary:

```sh
find "$CED2_VERIFY_ROOT/db" -type f -name '*.json' -exec shasum -a 256 {} \; \
  | sort > "$CED2_VERIFY_ROOT/after.sha256"
diff -u "$CED2_VERIFY_ROOT/before.sha256" "$CED2_VERIFY_ROOT/after.sha256"
```

Only `maps.json` should differ. Remove `CED2_VERIFY_ROOT` after keeping any
screenshots, traces, or hash reports that are needed.

## Functional matrix

Use Chrome device emulation to run the complete matrix once at DPR 1 and once
at DPR 2. Keep the CSS viewport size identical between runs.

| Check            | Action                                                                   | Required observation                                                                                       |
| ---------------- | ------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------- |
| Initial render   | Wait for the frame status to show drawn/visible counts                   | No canvas error; sprites are crisp; no gaps or overlaps at map seams                                       |
| DPR backing      | Inspect the canvas with the console expression below                     | Backing dimensions are rounded CSS dimensions multiplied by DPR                                            |
| Pointer          | Hover and select a known cell before and after a window resize           | Inspector map, index, and `(x, y)` match the visible cell at both DPRs                                     |
| Zoom             | Wheel in and out over a cell                                             | The same world cell remains under the pointer; scale remains within 0.5–10                                 |
| Pan              | Middle-drag, then hover the original cell at its new location            | View moves by the drag delta and hit testing stays aligned                                                 |
| Layer            | Exercise layers `1`, `0`, and `-1`                                       | Each layer renders without normalization or loss of the negative layer                                     |
| Grid render      | Pan several partitions away from the initially focused map               | Visible partitions stream into the scene with one-partition overscan and without blank seams               |
| Grid access      | Hover and edit any compatible visible partition                          | Inspector reports its backing map and says `editable`; no focused-partition switch is required             |
| Cross-map pencil | Select `terrain0`, then drag from the focused map into its west neighbor | One continuous gesture paints both blocks; the inspector changes map identity at the seam                  |
| Translation      | Inspect the saved west-neighbor pair                                     | `terrain0` is index 4 in the focus but index 2 in `alinea_outsideAlinea2`; the neighbor must store index 2 |
| Undo/redo        | Undo once, then redo once after the cross-map stroke                     | One undo restores every cell in both maps; one redo reapplies all of them                                  |
| Erase            | Erase a short path crossing the same seam, then undo                     | Both maps erase to `[0, 0]`; one undo restores both sides                                                  |
| Save/reload      | Click Save All, verify the temporary hashes, then reload                 | Only `maps.json` changes and the edited cells survive reload                                               |
| Error surface    | Watch DevTools Console and Network throughout                            | No uncaught exception, failed API/media request, or persistent render error                                |

Canvas backing inspection:

```js
(() => {
  const canvas = document.querySelector('.map-canvas');
  const rect = canvas.getBoundingClientRect();
  return {
    dpr: devicePixelRatio,
    css: [rect.width, rect.height],
    expectedBacking: [
      Math.round(rect.width * Math.min(4, Math.max(1, devicePixelRatio))),
      Math.round(rect.height * Math.min(4, Math.max(1, devicePixelRatio))),
    ],
    actualBacking: [canvas.width, canvas.height],
  };
})();
```

For a useful real-data spot check, also open `Alinea1` (40×40 and four local
tilesets) and `alinea_insideAlinea7` (three layers including `-1`). Compare
ground patterns, structure edges, blank tiles, layer selection, and unresolved
sprite counts.

## Legacy visual comparison

Run legacy `ceditor` separately because it uses the same ports. It may read the
live data for comparison, but do not invoke any save action. Open the same map
and layer in each editor, use the same approximate scale, and capture full-page
screenshots.

Compare:

- tile identity and orientation at at least five landmarks;
- row/column alignment and seams between neighboring maps;
- blank cells and map boundaries;
- layer `1`, `0`, and `-1` content;
- nearest-neighbor pixel appearance at integer and fractional zoom;
- missing-image or unresolved-sprite indicators.

Metadata overlays and later-phase tools are not Phase 4 visual regressions;
record them as parity backlog rather than failing this kernel gate.

## Browser performance trace

Use a normal interactive browser, not a managed headless session:

1. Close unrelated tabs and keep viewport, DPR, browser version, and selected
   map constant for both editors.
2. In DevTools Performance, enable screenshots and record a reload through the
   first useful sprite frame. Record navigation start, database response end,
   sprite-sheet response end, first canvas paint, and first useful map frame.
3. Record five idle seconds on `Alinea1`, then five seconds containing a steady
   middle-button pan and wheel zoom. Repeat three times.
4. Record five idle seconds on `alinea_outsideAlinea1`, pan several partitions,
   then paint a stroke across a seam. Repeat three times.
5. Preserve the trace and report median frame duration, p95 frame duration,
   long tasks over 50 ms, main-thread utilization, and dropped frames. Note the
   browser/GPU and whether the run used DPR 1 or 2.
6. Compare medians across repeated runs. Do not accept an optimization based on
   one maximum, one trace, or a Node-only microbenchmark.

The continuous animation loop is intentional. Idle frame activity is expected;
the questions are whether it stays within the frame budget, avoids long tasks,
and remains responsive during pan, zoom, and cross-map painting.

## Local browser-tool audit and attempted automation

On September 10, 2026 this workstation had:

- Google Chrome 152.0.7977.83 extended;
- Firefox 154.0;
- Safari installed;
- no Playwright, Puppeteer, Selenium WebDriver, ChromeDriver, or GeckoDriver in
  the project or executable path.

A dependency-free temporary Chrome DevTools Protocol driver was tried against
an isolated database. Chrome launched and loaded the Vite page. From the Node
process, both `http://127.0.0.1:3001/api/database` and Vite's proxied
`http://127.0.0.1:3000/api/database` succeeded; the managed headless page's own
fetch failed and rendered `Could not reach the database server`. Chrome also
reported unavailable macOS CVDisplayLink and keychain services. Therefore no
browser screenshot or browser frame timing is claimed as completed evidence
from that run.

The temporary driver grew to roughly 1,000 lines because it had to implement
CDP transport, lifecycle, input, isolation, and reporting without a browser
library. It was removed: that maintenance cost conflicts with CEditor2's
simplicity goal, especially when the local managed-headless environment cannot
produce a valid run. The focused automated tests plus this concise interactive
protocol are the current verification boundary.

## Evidence record

Attach one record to the Phase 4 checkpoint:

| Field                               | Value                      |
| ----------------------------------- | -------------------------- |
| Commit                              |                            |
| Date/machine                        |                            |
| Browser/GPU                         |                            |
| Viewport and DPR                    |                            |
| Maps/layers checked                 |                            |
| DPR 1 backing/pointer               | pass/fail                  |
| DPR 2 backing/pointer               | pass/fail                  |
| Neighbor render/access              | pass/fail                  |
| Cross-map paint/erase/undo/redo     | pass/fail                  |
| Save/reload and changed-file hashes | pass/fail                  |
| Legacy screenshot comparison        | pass/fail + artifact paths |
| First useful frame                  | three runs + median        |
| Idle frame p50/p95                  | three runs + median        |
| Interactive frame p50/p95           | three runs + median        |
| Long tasks/dropped frames           |                            |
| Console/network errors              |                            |
| Trace and screenshot paths          |                            |
