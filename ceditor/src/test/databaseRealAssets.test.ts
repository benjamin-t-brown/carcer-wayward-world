import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import {
  cp,
  mkdtemp,
  readFile,
  readdir,
  rename,
  rm,
  stat,
} from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import test from 'node:test';

import {
  DatabaseRepository,
  type DatabaseCommit,
} from '../server/databaseRepository';
import { commitSaveTransaction } from '../server/saveTransaction';
import { ASSET_TYPES } from '../shared/assetRegistry';
import type {
  DatabaseSnapshot,
  JsonArray,
  SaveDatabaseRequest,
} from '../shared/databaseContract';

const REPOSITORY_DATABASE_PATH = resolve(
  dirname(fileURLToPath(import.meta.url)),
  '../../../src/assets/db',
);

interface FileState {
  content: Buffer;
  inode: number;
  modifiedMs: number;
}

test('real database golden loads every managed file without normalization', async () => {
  await withRealDatabaseCopy(async (databasePath, sourceContents) => {
    const repository = new DatabaseRepository(databasePath);
    const loaded = await repository.load();
    const expectedAssets = {} as DatabaseSnapshot;

    for (const { id, file } of ASSET_TYPES) {
      const content = sourceContents.get(file);
      assert.ok(content, `missing real database golden ${file}`);
      const parsed: unknown = JSON.parse(content.toString('utf8'));
      assert.ok(Array.isArray(parsed), `${file} must have an array root`);
      expectedAssets[id] = parsed as JsonArray;
    }

    assert.deepEqual(
      Object.keys(loaded.assets),
      ASSET_TYPES.map(({ id }) => id),
    );
    assert.deepEqual(loaded.assets, expectedAssets);
    assert.equal(loaded.revision, revisionForGolden(sourceContents));
  });
});

test('real database no-op save preserves every copied byte, inode, and mtime', async () => {
  await withRealDatabaseCopy(async (databasePath) => {
    const repository = new DatabaseRepository(databasePath);
    const loaded = await repository.load();
    const before = await readTreeState(databasePath);

    const result = await repository.save({
      baseRevision: loaded.revision,
      assets: structuredClone(loaded.assets),
    });

    assert.deepEqual(result, {
      revision: loaded.revision,
      changedFiles: [],
    });
    assert.deepEqual(await readTreeState(databasePath), before);
    assert.deepEqual(await transactionDirectories(databasePath), []);
  });
});

test('real database copy rolls back a repository commit failure and permits retry', async () => {
  await withRealDatabaseCopy(async (databasePath) => {
    let failReplacement = true;
    let replacementCount = 0;
    const commit: DatabaseCommit = (options) =>
      commitSaveTransaction({
        ...options,
        replaceFile: async (source, destination) => {
          replacementCount++;
          if (failReplacement && replacementCount === 2) {
            throw new Error('injected real-database replacement failure');
          }
          await rename(source, destination);
        },
      });
    const repository = new DatabaseRepository(databasePath, commit);
    const loaded = await repository.load();
    const request = changedRealDatabaseRequest(loaded.revision, loaded.assets);
    const beforeFailure = await readTreeContents(databasePath);

    await assert.rejects(
      repository.save(request),
      /injected real-database replacement failure/,
    );

    assert.deepEqual(await readTreeContents(databasePath), beforeFailure);
    assert.deepEqual(await transactionDirectories(databasePath), []);
    assert.equal((await repository.load()).revision, loaded.revision);

    failReplacement = false;
    replacementCount = 0;
    const retry = await repository.save(request);
    assert.deepEqual(retry.changedFiles, ['items.json', 'abilities.json']);
    assert.notEqual(retry.revision, loaded.revision);

    const afterRetry = await readTreeContents(databasePath);
    for (const { file } of ASSET_TYPES) {
      if (file !== 'items.json' && file !== 'abilities.json') {
        assert.deepEqual(afterRetry.get(file), beforeFailure.get(file));
      }
    }
    for (const [relativePath, content] of beforeFailure) {
      if (!ASSET_TYPES.some(({ file }) => file === relativePath)) {
        assert.deepEqual(afterRetry.get(relativePath), content);
      }
    }
    assert.deepEqual(await transactionDirectories(databasePath), []);
  });
});

function changedRealDatabaseRequest(
  baseRevision: string,
  assets: DatabaseSnapshot,
): SaveDatabaseRequest {
  const changed = structuredClone(assets);
  changed.itemTemplates.push({ name: '__CEDITOR_TRANSACTION_TEST_ITEM__' });
  changed.abilityTemplates.push({
    name: '__CEDITOR_TRANSACTION_TEST_ABILITY__',
  });
  return { baseRevision, assets: changed };
}

async function withRealDatabaseCopy(
  run: (
    databasePath: string,
    sourceContents: ReadonlyMap<string, Buffer>,
  ) => Promise<void>,
): Promise<void> {
  const sourceContents = await readTreeContents(REPOSITORY_DATABASE_PATH);
  const temporaryRoot = await mkdtemp(join(tmpdir(), 'ceditor-real-db-'));
  const databasePath = join(temporaryRoot, 'db');

  try {
    await cp(REPOSITORY_DATABASE_PATH, databasePath, {
      recursive: true,
      preserveTimestamps: true,
    });
    await run(databasePath, sourceContents);
  } finally {
    const sourceAfter = await readTreeContents(REPOSITORY_DATABASE_PATH);
    await rm(temporaryRoot, { recursive: true, force: true });
    assert.deepEqual(
      sourceAfter,
      sourceContents,
      'real repository database assets changed during integration coverage',
    );
  }
}

async function readTreeContents(root: string): Promise<Map<string, Buffer>> {
  const state = await readTreeState(root);
  return new Map(
    [...state].map(([relativePath, file]) => [relativePath, file.content]),
  );
}

async function readTreeState(root: string): Promise<Map<string, FileState>> {
  const files: Array<[string, FileState]> = [];

  const visit = async (directory: string, relativeDirectory = '') => {
    const entries = await readdir(directory, { withFileTypes: true });
    for (const entry of entries.sort((a, b) => a.name.localeCompare(b.name))) {
      const relativePath = relativeDirectory
        ? join(relativeDirectory, entry.name)
        : entry.name;
      const path = join(directory, entry.name);
      if (entry.isDirectory()) {
        await visit(path, relativePath);
        continue;
      }
      if (!entry.isFile()) continue;
      const [content, info] = await Promise.all([readFile(path), stat(path)]);
      files.push([
        relativePath,
        { content, inode: info.ino, modifiedMs: info.mtimeMs },
      ]);
    }
  };

  await visit(root);
  return new Map(files);
}

async function transactionDirectories(databasePath: string): Promise<string[]> {
  return (await readdir(databasePath))
    .filter((name) => name.startsWith('.ceditor-save-'))
    .sort();
}

function revisionForGolden(contents: ReadonlyMap<string, Buffer>): string {
  const hash = createHash('sha256');
  for (const { file } of ASSET_TYPES) {
    const content = contents.get(file);
    assert.ok(content, `missing managed file ${file}`);
    hash.update(file, 'utf8');
    hash.update('\0');
    hash.update(content);
    hash.update('\0');
  }
  return hash.digest('hex');
}
