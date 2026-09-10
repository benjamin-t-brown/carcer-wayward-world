import assert from 'node:assert/strict';
import test from 'node:test';

import {
  bumpMapDataRevision,
  clearRenderDirty,
  getMapDataRevision,
  isRenderDirty,
  registerMapDataRevision,
  renameMapDataRevision,
} from '../client/tile-editor/mapEditorSignals';

test('map revisions remain isolated from editor UI state and follow renames', () => {
  const originalName = 'signal-test-original';
  const renamedName = 'signal-test-renamed';

  clearRenderDirty();
  bumpMapDataRevision(originalName);
  assert.equal(getMapDataRevision(originalName), 0);
  assert.equal(isRenderDirty(), false);

  registerMapDataRevision(originalName);
  bumpMapDataRevision(originalName);
  assert.equal(getMapDataRevision(originalName), 1);
  assert.equal(isRenderDirty(), true);

  clearRenderDirty();
  renameMapDataRevision(originalName, renamedName);
  assert.equal(getMapDataRevision(originalName), 0);
  assert.equal(getMapDataRevision(renamedName), 1);

  bumpMapDataRevision(renamedName);
  assert.equal(getMapDataRevision(renamedName), 2);
  assert.equal(isRenderDirty(), true);
});
