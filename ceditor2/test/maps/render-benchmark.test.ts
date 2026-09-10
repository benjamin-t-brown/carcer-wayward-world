import assert from 'node:assert/strict';
import test from 'node:test';
import {
  MAP_BENCHMARK_SCENARIOS,
  benchmarkMapScenario,
  generateCompactGraphics,
  summarizeFrameTimings,
} from '../../bench/map-render-harness.js';

test('compact benchmark maps have a deterministic blank/drawn/unresolved mix', () => {
  const graphics = generateCompactGraphics(10, 10);
  assert.equal(graphics.byteLength, 400);
  assert.deepEqual(
    [...graphics.slice(0, 12)],
    [0, 0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 5],
  );
  assert.deepEqual([...graphics.slice(104, 110)], [1, 52, 2, 53, 1, 54]);
});

test('representative scenarios have stable culling counts and visual traces', async () => {
  const expected = {
    'small-full-map': {
      visited: 432,
      drawn: 398,
      blank: 26,
      unresolved: 8,
      visualHash: '4f7d03c8',
    },
    'medium-panned': {
      visited: 840,
      drawn: 777,
      blank: 50,
      unresolved: 13,
      visualHash: 'a2d8b091',
    },
    'large-zoomed-out': {
      visited: 9940,
      drawn: 9178,
      blank: 584,
      unresolved: 178,
      visualHash: '37180b52',
    },
  } as const;

  for (const scenario of MAP_BENCHMARK_SCENARIOS) {
    const result = await benchmarkMapScenario(scenario, {
      warmupFrames: 0,
      measuredFrames: 1,
    });
    const wanted = expected[scenario.id as keyof typeof expected];
    assert.equal(result.renderStats.visitedTiles, wanted.visited, scenario.id);
    assert.equal(result.renderStats.drawnTiles, wanted.drawn, scenario.id);
    assert.equal(result.renderStats.blankTiles, wanted.blank, scenario.id);
    assert.equal(
      result.renderStats.unresolvedSprites,
      wanted.unresolved,
      scenario.id,
    );
    assert.equal(result.renderStats.pendingImages, 0, scenario.id);
    assert.equal(result.canvasOperations.drawImages, wanted.drawn, scenario.id);
    assert.equal(result.canvasOperations.smoothingDisabled, true, scenario.id);
    assert.equal(result.visualHash, wanted.visualHash, scenario.id);
  }
});

test('timing summaries use nearest-rank percentiles independently of the clock', () => {
  assert.deepEqual(summarizeFrameTimings([5, 1, 4, 2, 3]), {
    averageMs: 3,
    minMs: 1,
    p50Ms: 3,
    p95Ms: 5,
    p99Ms: 5,
    maxMs: 5,
  });
  assert.deepEqual(summarizeFrameTimings([]), {
    averageMs: 0,
    minMs: 0,
    p50Ms: 0,
    p95Ms: 0,
    p99Ms: 0,
    maxMs: 0,
  });
});
