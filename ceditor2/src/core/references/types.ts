import type { AssetId } from '../database/assetRegistry.js';

export type ReferenceTargetKind =
  AssetId | 'mapOrMapGrid' | 'mapMarker' | 'eventNode';

export type ReferenceIntegrity = 'hard' | 'soft' | 'structural';

export interface ReferenceSource {
  assetId: AssetId;
  recordId: string;
  path: string;
}

export interface ReferenceTarget {
  kind: ReferenceTargetKind;
  id: string;
  /** Map name for a marker, or event id for an event node. */
  scope?: string;
}

export interface DatabaseReference {
  relation: string;
  source: ReferenceSource;
  target: ReferenceTarget;
  integrity: ReferenceIntegrity;
  resolved: boolean;
}

export interface ReferenceTargetQuery {
  kind: ReferenceTargetKind;
  id: string;
  scope?: string;
}
