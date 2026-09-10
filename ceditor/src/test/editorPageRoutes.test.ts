import assert from 'node:assert/strict';
import test from 'node:test';

import { ASSET_TYPES } from '../shared/assetRegistry';
import {
  EDITOR_PAGE_BY_ASSET_ID,
  SOUND_EFFECTS_ROUTE,
  editorPageForRoute,
} from '../client/editorPageRoutes';
import { editorRouteForAssetId } from '../client/utils/editorRoutes';

test('every managed asset route resolves to its lazy editor page', () => {
  assert.deepEqual(
    Object.keys(EDITOR_PAGE_BY_ASSET_ID).sort(),
    ASSET_TYPES.map(({ id }) => id).sort(),
  );

  for (const { id } of ASSET_TYPES) {
    const route = editorRouteForAssetId(id);
    assert.ok(route);
    assert.equal(editorPageForRoute(route), EDITOR_PAGE_BY_ASSET_ID[id]);
  }
});

test('sound effects resolve separately and unknown paths remain unmatched', () => {
  assert.ok(editorPageForRoute(SOUND_EFFECTS_ROUTE));
  assert.equal(editorPageForRoute('/editor/not-managed'), undefined);
});
