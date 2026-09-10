import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

const EXPECTED_ENTRIES = [
  'abilities',
  'characters',
  'events',
  'items',
  'map-grids',
  'maps',
  'sounds',
  'spells',
  'status-effects',
  'tilesets',
] as const;

test('the browser runtime has no UI, state, form, router, or canvas framework', async () => {
  const manifest = JSON.parse(await readFile('package.json', 'utf8')) as {
    dependencies?: Record<string, string>;
  };

  assert.deepEqual(manifest.dependencies, { express: '5.2.1' });
});

test('every retained editor has an independent HTML entry', async () => {
  await Promise.all(
    EXPECTED_ENTRIES.map(async (name) => {
      const html = await readFile(`pages/${name}/index.html`, 'utf8');
      assert.match(html, /<script type="module"/);
      assert.match(html, new RegExp(`src/apps/${name}/index\\.ts`));
    }),
  );
});
