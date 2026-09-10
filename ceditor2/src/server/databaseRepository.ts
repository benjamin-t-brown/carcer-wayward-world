import { createHash } from 'node:crypto';
import { readFile } from 'node:fs/promises';
import { join } from 'node:path';
import { isDeepStrictEqual } from 'node:util';

import { ASSET_REGISTRY } from '../core/database/assetRegistry.js';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  JsonArray,
  SaveDatabaseRequest,
  SaveDatabaseResponse,
} from '../core/database/types.js';
import { validateDatabase } from '../core/validation/index.js';
import { BadRequestError, ConflictError } from './httpErrors.js';
import { commitSaveTransaction, type SaveOutput } from './saveTransaction.js';

interface DiskDatabase {
  assets: DatabaseSnapshot;
  contents: Map<string, string>;
  revision: string;
}

export interface DatabaseRepositoryContract {
  load(): Promise<DatabaseEnvelope>;
  save(request: unknown): Promise<SaveDatabaseResponse>;
}

export class DatabaseRepository implements DatabaseRepositoryContract {
  private pendingSave: Promise<void> = Promise.resolve();

  constructor(readonly databasePath: string) {}

  async load(): Promise<DatabaseEnvelope> {
    const disk = await this.readDiskDatabase();
    return { revision: disk.revision, assets: disk.assets };
  }

  async save(request: unknown): Promise<SaveDatabaseResponse> {
    const validated = parseSaveRequest(request);
    return this.withSaveLock(() => this.saveLocked(validated));
  }

  private async saveLocked(
    request: SaveDatabaseRequest,
  ): Promise<SaveDatabaseResponse> {
    const current = await this.readDiskDatabase();
    if (current.revision !== request.baseRevision) {
      throw new ConflictError(
        'The database changed on disk. Reload before saving again.',
      );
    }

    const validation = validateDatabase(request.assets);
    if (!validation.valid) {
      const details = validation.errors
        .map((issue) => `${issue.path}: ${issue.message}`)
        .join('\n');
      throw new BadRequestError(`Database validation failed:\n${details}`);
    }

    const outputs = serializeSnapshot(request.assets).map((output) => {
      const definition = ASSET_REGISTRY.find(
        ({ fileName }) => fileName === output.fileName,
      )!;
      const original = current.contents.get(output.fileName)!;
      return isDeepStrictEqual(
        request.assets[definition.id],
        current.assets[definition.id],
      )
        ? { ...output, content: original }
        : output;
    });
    const changedFiles = await commitSaveTransaction({
      databasePath: this.databasePath,
      outputs,
      currentContents: current.contents,
    });

    return {
      revision: revisionForOutputs(outputs),
      changedFiles,
    };
  }

  private async readDiskDatabase(): Promise<DiskDatabase> {
    const entries = await Promise.all(
      ASSET_REGISTRY.map(async (definition) => {
        const content = await readFile(
          join(this.databasePath, definition.fileName),
          'utf8',
        );
        let parsed: unknown;
        try {
          parsed = JSON.parse(content);
        } catch (error) {
          throw new Error(
            `Failed to parse managed database file ${definition.fileName}`,
            { cause: error },
          );
        }
        if (!Array.isArray(parsed)) {
          throw new Error(
            `Managed database file must have an array root: ${definition.fileName}`,
          );
        }
        return { definition, content, records: parsed as JsonArray };
      }),
    );

    const assets = {} as DatabaseSnapshot;
    const contents = new Map<string, string>();
    for (const { definition, content, records } of entries) {
      assets[definition.id] = records;
      contents.set(definition.fileName, content);
    }

    return {
      assets,
      contents,
      revision: revisionForContents(contents),
    };
  }

  private async withSaveLock<T>(operation: () => Promise<T>): Promise<T> {
    const preceding = this.pendingSave;
    let release: (() => void) | undefined;
    this.pendingSave = new Promise<void>((resolve) => {
      release = resolve;
    });

    await preceding;
    try {
      return await operation();
    } finally {
      release?.();
    }
  }
}

export function parseSaveRequest(request: unknown): SaveDatabaseRequest {
  if (!isJsonObject(request)) {
    throw new BadRequestError('Request body must be an object.');
  }
  if (typeof request.baseRevision !== 'string' || !request.baseRevision) {
    throw new BadRequestError('baseRevision must be a non-empty string.');
  }
  if (!isJsonObject(request.assets)) {
    throw new BadRequestError('assets must be an object.');
  }

  const managedIds = new Set<string>(
    ASSET_REGISTRY.map((definition) => definition.id),
  );
  const unexpectedIds = Object.keys(request.assets).filter(
    (id) => !managedIds.has(id),
  );
  if (unexpectedIds.length > 0) {
    throw new BadRequestError(
      `Unknown asset collections: ${unexpectedIds.join(', ')}.`,
    );
  }

  const assets = {} as DatabaseSnapshot;
  for (const definition of ASSET_REGISTRY) {
    const records = request.assets[definition.id];
    if (!Array.isArray(records)) {
      throw new BadRequestError(`assets.${definition.id} must be an array.`);
    }
    assets[definition.id] = records;
  }

  return { baseRevision: request.baseRevision, assets };
}

export function serializeSnapshot(snapshot: DatabaseSnapshot): SaveOutput[] {
  return ASSET_REGISTRY.map(({ id, fileName }) => ({
    fileName,
    content: `${JSON.stringify(snapshot[id], null, 2)}\n`,
  }));
}

function revisionForContents(contents: ReadonlyMap<string, string>): string {
  const hash = createHash('sha256');
  for (const { fileName } of ASSET_REGISTRY) {
    const content = contents.get(fileName);
    if (content === undefined) {
      throw new Error(`Missing revision content for ${fileName}`);
    }
    updateRevisionHash(hash, fileName, content);
  }
  return hash.digest('hex');
}

function revisionForOutputs(outputs: readonly SaveOutput[]): string {
  const byFile = new Map(
    outputs.map(({ fileName, content }) => [fileName, content]),
  );
  return revisionForContents(byFile);
}

function updateRevisionHash(
  hash: ReturnType<typeof createHash>,
  fileName: string,
  content: string,
): void {
  hash.update(fileName, 'utf8');
  hash.update('\0');
  hash.update(content, 'utf8');
  hash.update('\0');
}

function isJsonObject(value: unknown): value is Record<string, unknown> {
  return (
    typeof value === 'object' &&
    value !== null &&
    !Array.isArray(value) &&
    Object.getPrototypeOf(value) === Object.prototype
  );
}
