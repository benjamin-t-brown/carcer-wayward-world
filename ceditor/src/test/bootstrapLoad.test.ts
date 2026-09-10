import assert from 'node:assert/strict';
import test from 'node:test';

import { loadEditorData } from '../client/bootstrap/loadEditorData';
import { ASSET_TYPES } from '../shared/assetRegistry';
import type { DatabaseSnapshot, JsonArray } from '../shared/databaseContract';

test('bootstrap performs one database GET and normalizes a defensive copy', async () => {
  const assets = emptySnapshot();
  assets.spellTemplates = [
    {
      name: 'spark',
      label: 'Spark',
      description: '',
      icon: 'icons_0',
      abilityName: '',
      requiredRunes: [{ type: 'HEAT', count: 2.8 }],
      futureField: 'preserved',
    },
  ];
  const requests: Array<{ url: string; method: string }> = [];
  const fetchImplementation = async (
    input: string | URL | Request,
    init?: RequestInit,
  ): Promise<Response> => {
    const url =
      typeof input === 'string'
        ? input
        : input instanceof URL
          ? input.href
          : input.url;
    requests.push({ url, method: init?.method ?? 'GET' });

    if (url === '/api/database') {
      return Response.json({ revision: 'revision-1', assets });
    }
    if (url === '/api/sdl2w-assets') {
      return Response.json({
        'assets.test.txt': [
          'Pic,icons,assets/img/icons.png',
          'Sprites,icons,1,16,16',
          'Sound,ping,assets/snd/ping',
          'Anim,flash,loop',
          'icons_0,2',
          'EndAnim',
        ].join('\n'),
      });
    }
    return new Response('Not Found', { status: 404 });
  };

  const data = await loadEditorData({
    fetch: fetchImplementation as typeof globalThis.fetch,
  });

  assert.deepEqual(requests, [
    { url: '/api/database', method: 'GET' },
    { url: '/api/sdl2w-assets', method: 'GET' },
  ]);
  assert.equal(data.databaseSession.baseRevision, 'revision-1');
  assert.equal(data.assetTypes.length, 9);
  assert.equal(data.sprites[0]?.name, 'icons_0');
  assert.equal(data.spriteMap.icons_0, data.sprites[0]);
  assert.equal(data.animationMap.flash, data.animations[0]);
  assert.equal(data.soundMap.ping, data.sounds[0]);
  assert.deepEqual(data.spells[0].requiredRunes, [{ type: 'HEAT', count: 2 }]);

  data.spells[0].name = 'changed-in-editor-copy';
  assert.equal(
    (data.databaseSession.snapshot().spellTemplates[0] as { name: string })
      .name,
    'spark',
  );
});

function emptySnapshot(): DatabaseSnapshot {
  const snapshot = {} as DatabaseSnapshot;
  for (const { id } of ASSET_TYPES) {
    snapshot[id] = [] as JsonArray;
  }
  return snapshot;
}
