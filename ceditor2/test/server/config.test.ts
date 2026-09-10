import assert from 'node:assert/strict';
import test from 'node:test';

import {
  DEFAULT_SERVER_PORT,
  resolveServerPort,
} from '../../src/server/config.js';

test('server port defaults to 3001', () => {
  assert.equal(resolveServerPort(undefined), DEFAULT_SERVER_PORT);
  assert.equal(resolveServerPort(''), DEFAULT_SERVER_PORT);
  assert.equal(resolveServerPort('  '), DEFAULT_SERVER_PORT);
});

test('server port accepts valid integer strings', () => {
  assert.equal(resolveServerPort('1'), 1);
  assert.equal(resolveServerPort('3001'), 3001);
  assert.equal(resolveServerPort('65535'), 65_535);
});

test('server port rejects invalid values', () => {
  for (const value of ['0', '65536', '-1', '1.5', 'not-a-port']) {
    assert.throws(() => resolveServerPort(value), /must be an integer/);
  }
});
