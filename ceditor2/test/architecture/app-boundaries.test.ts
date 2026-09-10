import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import path from 'node:path';
import test from 'node:test';

const APPS_ROOT = path.resolve('src/apps');

async function collectTypeScriptFiles(directory: string): Promise<string[]> {
  const entries = await readdir(directory, { withFileTypes: true });
  const files = await Promise.all(
    entries.map(async (entry) => {
      const entryPath = path.join(directory, entry.name);
      if (entry.isDirectory()) {
        return collectTypeScriptFiles(entryPath);
      }
      return entry.name.endsWith('.ts') ? [entryPath] : [];
    }),
  );
  return files.flat();
}

test('applications do not import other applications', async () => {
  const appDirectories = (await readdir(APPS_ROOT, { withFileTypes: true }))
    .filter((entry) => entry.isDirectory())
    .map((entry) => entry.name);

  for (const app of appDirectories) {
    const files = await collectTypeScriptFiles(path.join(APPS_ROOT, app));
    for (const file of files) {
      const source = await readFile(file, 'utf8');
      const importPattern = /\b(?:from|import)\s*(?:\(\s*)?['"]([^'"]+)['"]/g;
      for (const match of source.matchAll(importPattern)) {
        const specifier = match[1];
        if (!specifier?.startsWith('.')) {
          continue;
        }
        const target = path.resolve(path.dirname(file), specifier);
        const relativeTarget = path.relative(APPS_ROOT, target);
        if (relativeTarget.startsWith('..')) {
          continue;
        }
        const [targetApp] = relativeTarget.split(path.sep);
        assert.equal(
          targetApp,
          app,
          `${path.relative(APPS_ROOT, file)} imports the ${targetApp} app`,
        );
      }
    }
  }
});
