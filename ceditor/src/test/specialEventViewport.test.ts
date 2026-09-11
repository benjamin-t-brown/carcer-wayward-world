import assert from 'node:assert/strict';
import test from 'node:test';
import {
  calculateVisibleEventWorldRect,
  connectorIntersectsWorldRect,
  planSpecialEventViewport,
} from '../client/special-event-editor/specialEventViewport';

type TestConnector = {
  id: string;
  fromNodeId: string;
  toNodeId: string;
  segments: Array<{ x: number; y: number }>;
};

type TestNode = {
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  exits: TestConnector[];
};

test('visible event world bounds exactly reverse pan, zoom, and centering', () => {
  assert.deepEqual(
    calculateVisibleEventWorldRect({
      canvasWidth: 1000,
      canvasHeight: 500,
      zoneWidth: 8000,
      zoneHeight: 8000,
      transform: { translateX: 100, translateY: -50, scale: 2 },
    }),
    { minX: 3450, minY: 3775, maxX: 3950, maxY: 4025 },
  );

  const zoomedOut = calculateVisibleEventWorldRect({
    canvasWidth: 1000,
    canvasHeight: 500,
    zoneWidth: 8000,
    zoneHeight: 8000,
    transform: { translateX: 0, translateY: 0, scale: 0.5 },
  });
  assert.equal(zoomedOut.maxX - zoomedOut.minX, 2000);
  assert.equal(zoomedOut.maxY - zoomedOut.minY, 1000);
});

test('viewport planning retains source order while culling distant nodes', () => {
  const nodes: TestNode[] = Array.from({ length: 20_000 }, (_, index) => ({
    id: `distant-${index}`,
    x: -100_000 - index * 10,
    y: -100_000,
    width: 5,
    height: 5,
    exits: [],
  }));
  nodes.splice(
    10_000,
    0,
    {
      id: 'visible-first',
      x: 475,
      y: 480,
      width: 20,
      height: 20,
      exits: [],
    },
    {
      id: 'visible-second',
      x: 510,
      y: 510,
      width: 20,
      height: 20,
      exits: [],
    },
  );

  const plan = planSpecialEventViewport({
    canvasWidth: 100,
    canvasHeight: 100,
    zoneWidth: 1000,
    zoneHeight: 1000,
    transform: { translateX: 0, translateY: 0, scale: 1 },
    nodes,
  });

  assert.equal(nodes.length, 20_002);
  assert.deepEqual(
    plan.nodes.map((node) => node.id),
    ['visible-first', 'visible-second'],
  );
});

test('visible connectors survive when both endpoint nodes are outside the viewport', () => {
  const crossing: TestConnector = {
    id: 'crossing',
    fromNodeId: 'left',
    toNodeId: 'right',
    segments: [
      { x: 100, y: 500 },
      { x: 900, y: 500 },
    ],
  };
  const distant: TestConnector = {
    id: 'distant',
    fromNodeId: 'left',
    toNodeId: 'right',
    segments: [
      { x: 100, y: 100 },
      { x: 200, y: 100 },
    ],
  };
  const nodes: TestNode[] = [
    {
      id: 'left',
      x: 100,
      y: 490,
      width: 20,
      height: 20,
      exits: [crossing, distant],
    },
    {
      id: 'right',
      x: 900,
      y: 490,
      width: 20,
      height: 20,
      exits: [],
    },
  ];

  const plan = planSpecialEventViewport({
    canvasWidth: 100,
    canvasHeight: 100,
    zoneWidth: 1000,
    zoneHeight: 1000,
    transform: { translateX: 0, translateY: 0, scale: 1 },
    nodes,
  });

  assert.deepEqual(plan.nodes, []);
  assert.deepEqual(plan.connectors, [crossing]);
  assert.equal(
    connectorIntersectsWorldRect(distant, plan.visibleWorldRect),
    false,
  );
});

test('unlinked connector geometry is never scheduled for drawing', () => {
  const connector: TestConnector = {
    id: 'unlinked',
    fromNodeId: 'source',
    toNodeId: '',
    segments: [
      { x: 490, y: 500 },
      { x: 510, y: 500 },
    ],
  };

  assert.equal(
    connectorIntersectsWorldRect(connector, {
      minX: 450,
      minY: 450,
      maxX: 550,
      maxY: 550,
    }),
    false,
  );
});
