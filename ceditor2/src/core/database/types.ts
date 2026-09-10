import type { AssetId } from './assetRegistry.js';

export type JsonPrimitive = boolean | number | string | null;
export type JsonValue = JsonPrimitive | JsonObject | JsonArray;
export type JsonObject = { [key: string]: JsonValue };
export type JsonArray = JsonValue[];

export type DatabaseSnapshot = Record<AssetId, JsonArray>;

export interface DatabaseEnvelope {
  revision: string;
  assets: DatabaseSnapshot;
}

export interface SaveDatabaseRequest {
  baseRevision: string;
  assets: DatabaseSnapshot;
}

export interface SaveDatabaseResponse {
  revision: string;
  changedFiles: string[];
}

export interface DatabaseTransport {
  loadDatabase(): Promise<DatabaseEnvelope>;
  saveDatabase(request: SaveDatabaseRequest): Promise<SaveDatabaseResponse>;
}
