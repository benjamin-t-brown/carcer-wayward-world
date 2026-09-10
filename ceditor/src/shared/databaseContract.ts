import type { AssetId } from './assetRegistry';

export type JsonPrimitive = boolean | number | string | null;
export type JsonValue = JsonPrimitive | JsonObject | JsonArray;
export type JsonObject = { [key: string]: JsonValue };
export type JsonArray = JsonValue[];

/** A complete, coherent view of every JSON collection managed by CEditor. */
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
