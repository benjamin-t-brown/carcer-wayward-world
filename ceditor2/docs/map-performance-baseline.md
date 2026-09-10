# Map rendering performance baseline

## What this measures

`npm run benchmark:maps` runs `MapRenderer` headlessly in Node against three
generated, deterministic compact maps. It repeatedly calls `beginFrame` and
`drawMap`, which is equivalent to the work performed by a continuous animation
frame after `requestAnimationFrame` invokes the application.

The harness provides a narrow fake Canvas2D context and a preloaded fake image
cache. This deliberately measures renderer traversal, visible-cell culling,
sprite-sheet coordinate calculation, and Canvas2D command generation without
mixing in image decoding, browser layout, GPU rasterization, or compositing.
Those browser costs should be profiled separately when the editor is usable
end-to-end.

The generated map storage is a `Uint16Array` of maps.json-style
`[tilesetIndex, tileId]` pairs. Its reported byte count describes the benchmark
fixture, not the in-memory size of a parsed JSON number array.

## Scenarios

| Scenario         | Compact map | Canvas    | View                       | Cells visited | Cells drawn |
| ---------------- | ----------- | --------- | -------------------------- | ------------: | ----------: |
| Small full map   | 24×18       | 800×600   | Whole map at 1×            |           432 |         398 |
| Medium panned    | 128×128     | 1280×720  | Panned work area at 1.25×  |           840 |         777 |
| Large zoomed out | 512×512     | 1920×1080 | Interior work area at 0.5× |         9,940 |       9,178 |

Each scenario includes a one-tile culling margin and a stable mixture of drawn,
blank, and unresolved cells. The large map contains 262,144 total cells, but
the renderer visits only the 9,940 cells in the expanded visible bounds.

Deterministic counts and visual command hashes are asserted in
`test/maps/render-benchmark.test.ts`. Wall-clock values are reported but are
not pass/fail thresholds.

## Baseline run

Captured September 10, 2026 with the repository working tree on:

- Node v24.13.1
- macOS/Darwin 25.6.0, x64
- Intel Core i9-10910 at 3.60 GHz, 20 logical CPUs
- 100 warmup frames followed by 500 measured frames per scenario

| Scenario         |   Average |       p50 |       p95 |       p99 |   Maximum | Visual hash |
| ---------------- | --------: | --------: | --------: | --------: | --------: | ----------- |
| Small full map   | 0.0253 ms | 0.0158 ms | 0.0528 ms | 0.1129 ms | 0.1536 ms | `4f7d03c8`  |
| Medium panned    | 0.0369 ms | 0.0319 ms | 0.0768 ms | 0.0977 ms | 0.1519 ms | `a2d8b091`  |
| Large zoomed out | 0.3718 ms | 0.3448 ms | 0.4918 ms | 0.6070 ms | 0.7788 ms | `37180b52`  |

These times are a local reference, not a performance promise. Sub-millisecond
microbenchmarks are sensitive to CPU frequency, thermal state, background
processes, Node/V8 versions, and virtualization.

## Running and comparing

Human-readable output:

```sh
npm run benchmark:maps
```

Machine-readable output suitable for saving and comparing:

```sh
npm run benchmark:maps -- --json
```

The workload can be adjusted without changing the scenarios:

```sh
npm run benchmark:maps -- --warmup=200 --frames=2000 --json
```

For a useful before/after comparison:

1. Use the same machine, Node version, warmup count, and measured-frame count.
2. Run each revision several times while the machine is otherwise idle.
3. Compare `renderStats`, `canvasOperations`, and `visualHash` first. A change
   there means the renderer performed different work and should be understood
   before comparing speed.
4. Compare average and p50 for ordinary renderer cost, then p95/p99 for
   consistency. Treat isolated maximum values as environmental noise unless
   they recur.
5. Confirm browser behavior with the browser profiler before making design
   decisions from this Node-only benchmark.

No optimization should be accepted solely because one wall-clock run is
faster. The deterministic culling assertions remain the regression gate; the
timings are evidence for investigation.
