import { ASSET_IDS, type AssetId } from './assetRegistry.js';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  DatabaseTransport,
  JsonArray,
  SaveDatabaseResponse,
} from './types.js';

export class DatabaseSession {
  private revision: string;
  private assets: DatabaseSnapshot;
  private readonly dirty = new Set<AssetId>();
  private readonly changeVersions = new Map<AssetId, number>();
  private nextChangeVersion = 1;

  private constructor(
    private readonly client: DatabaseTransport,
    envelope: DatabaseEnvelope,
  ) {
    this.revision = envelope.revision;
    this.assets = structuredClone(envelope.assets);
  }

  static async load(client: DatabaseTransport): Promise<DatabaseSession> {
    const envelope = await client.loadDatabase();
    return new DatabaseSession(client, envelope);
  }

  get baseRevision(): string {
    return this.revision;
  }

  get isDirty(): boolean {
    return this.dirty.size > 0;
  }

  get dirtyAssetIds(): ReadonlySet<AssetId> {
    return new Set(this.dirty);
  }

  snapshot(): DatabaseSnapshot {
    return structuredClone(this.assets);
  }

  collection(id: AssetId): JsonArray {
    return structuredClone(this.assets[id]);
  }

  replaceCollection(id: AssetId, records: JsonArray): void {
    this.assets[id] = structuredClone(records);
    this.markDirty(id);
  }

  mutateCollection(id: AssetId, mutate: (records: JsonArray) => void): void {
    const records = structuredClone(this.assets[id]);
    mutate(records);
    this.assets[id] = records;
    this.markDirty(id);
  }

  async saveAll(): Promise<SaveDatabaseResponse> {
    const requestAssets = this.snapshot();
    const versionsAtStart = new Map(this.changeVersions);
    const response = await this.client.saveDatabase({
      baseRevision: this.revision,
      assets: requestAssets,
    });

    this.revision = response.revision;
    for (const id of ASSET_IDS) {
      if (this.changeVersions.get(id) === versionsAtStart.get(id)) {
        this.dirty.delete(id);
      }
    }

    return response;
  }

  private markDirty(id: AssetId): void {
    this.dirty.add(id);
    this.changeVersions.set(id, this.nextChangeVersion);
    this.nextChangeVersion += 1;
  }
}
