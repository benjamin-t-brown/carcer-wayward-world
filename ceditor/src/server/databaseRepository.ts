import { createHash } from 'crypto';
import { readFile } from 'fs/promises';
import { join } from 'path';
import { isDeepStrictEqual } from 'util';

import { ASSET_TYPES } from '../shared/assetRegistry';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  JsonArray,
  SaveDatabaseRequest,
  SaveDatabaseResponse,
} from '../shared/databaseContract';
import { BadRequestError, ConflictError } from './httpErrors';
import {
  commitSaveTransaction,
  type SaveOutput,
  type SaveTransactionOptions,
} from './saveTransaction';

interface DiskDatabase {
  assets: DatabaseSnapshot;
  contents: Map<string, string>;
  revision: string;
}

export interface DatabaseRepositoryContract {
  load(): Promise<DatabaseEnvelope>;
  save(request: unknown): Promise<SaveDatabaseResponse>;
}

export type DatabaseCommit = (
  options: SaveTransactionOptions,
) => Promise<string[]>;

export class DatabaseRepository implements DatabaseRepositoryContract {
  private pendingSave: Promise<void> = Promise.resolve();

  constructor(
    readonly databasePath: string,
    private readonly commit: DatabaseCommit = commitSaveTransaction,
  ) {}

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

    const outputs = serializeSnapshot(request.assets).map((output) => {
      const definition = ASSET_TYPES.find(
        ({ file }) => file === output.fileName,
      )!;
      const original = current.contents.get(output.fileName)!;

      return isDeepStrictEqual(
        request.assets[definition.id],
        current.assets[definition.id],
      )
        ? { ...output, content: original }
        : output;
    });

    const changedFiles = await this.commit({
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
      ASSET_TYPES.map(async (definition) => {
        const filePath = join(this.databasePath, definition.file);
        let content: string;
        try {
          content = await readFile(filePath, 'utf8');
        } catch (error) {
          throw new Error(
            `Failed to read managed database file ${definition.file}: ${errorMessage(error)}`,
          );
        }

        let parsed: unknown;
        try {
          parsed = JSON.parse(content);
        } catch (error) {
          throw new Error(
            `Failed to parse managed database file ${definition.file}: ${errorMessage(error)}`,
          );
        }
        if (!Array.isArray(parsed)) {
          throw new Error(
            `Managed database file must have an array root: ${definition.file}`,
          );
        }

        return {
          definition,
          content,
          records: parsed as JsonArray,
        };
      }),
    );

    const assets = {} as DatabaseSnapshot;
    const contents = new Map<string, string>();
    for (const { definition, content, records } of entries) {
      assets[definition.id] = records;
      contents.set(definition.file, content);
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

  const managedIds = new Set<string>(ASSET_TYPES.map(({ id }) => id));
  const unexpectedIds = Object.keys(request.assets).filter(
    (id) => !managedIds.has(id),
  );
  if (unexpectedIds.length > 0) {
    throw new BadRequestError(
      `Unknown asset collections: ${unexpectedIds.join(', ')}.`,
    );
  }

  const assets = {} as DatabaseSnapshot;
  for (const definition of ASSET_TYPES) {
    const records = request.assets[definition.id];
    if (!Array.isArray(records)) {
      throw new BadRequestError(`assets.${definition.id} must be an array.`);
    }
    assets[definition.id] = records;
  }

  return { baseRevision: request.baseRevision, assets };
}

export function serializeSnapshot(snapshot: DatabaseSnapshot): SaveOutput[] {
  return ASSET_TYPES.map(({ id, file }) => ({
    fileName: file,
    content: `${JSON.stringify(snapshot[id], null, 2)}\n`,
  }));
}

function revisionForContents(contents: ReadonlyMap<string, string>): string {
  const hash = createHash('sha256');
  for (const { file } of ASSET_TYPES) {
    const content = contents.get(file);
    if (content === undefined) {
      throw new Error(`Missing revision content for ${file}`);
    }
    updateRevisionHash(hash, file, content);
  }
  return hash.digest('hex');
}

function revisionForOutputs(outputs: readonly SaveOutput[]): string {
  return revisionForContents(
    new Map(outputs.map(({ fileName, content }) => [fileName, content])),
  );
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

function errorMessage(error: unknown): string {
  return error instanceof Error ? error.message : String(error);
}
