import assert from 'node:assert/strict';
import test from 'node:test';
import {
  ASSET_ROUTE_PARAM_BY_ID,
  ASSET_TYPES,
  isAssetId,
} from '../shared/assetRegistry';
import {
  assetIdForEditorRoute,
  editorRouteForAssetId,
} from '../client/utils/editorRoutes';

test('all nine managed assets have unique, reversible editor routes', () => {
  assert.equal(ASSET_TYPES.length, 9);

  const routes = ASSET_TYPES.map(({ id }) => {
    const route = editorRouteForAssetId(id);
    assert.ok(route, `${id} must have an editor route`);
    assert.equal(assetIdForEditorRoute(route), id);
    return route;
  });

  assert.equal(new Set(routes).size, routes.length);
});

test('unmanaged feats are neither routable nor accepted as an asset id', () => {
  assert.equal(isAssetId('featTemplates'), false);
  assert.equal(editorRouteForAssetId('featTemplates'), undefined);
});

test('status effects declare their deep-link query parameter', () => {
  assert.equal(ASSET_ROUTE_PARAM_BY_ID.statusEffectTemplates, 'statusEffect');
});
