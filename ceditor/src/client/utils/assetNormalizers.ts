/**
 * Per-asset-type normalization run on the client after loading raw JSON. Kept
 * on the client (not shared with the server) because it depends on the sprite /
 * animation / sound maps the server has no notion of.
 */
import { Animation, Sound } from './assetLoader';
import { sanitizeItemTemplates, sanitizeMapGridTemplates } from '../types/assets';
import { sanitizeAbilityTemplates } from '../types/ability';
import { sanitizeSpellTemplates } from '../types/spell';
import { normalizeMapItemsOnLoad } from '../tile-editor/mapTileItems';
import { normalizeMapOnLoad } from './mapIndex';
import { AssetId } from '../../shared/assetRegistry';

export interface LoadCtx {
  animationMap: Record<string, Animation>;
  soundMap: Record<string, Sound>;
  /** Outputs of already-run normalizers, for dependent normalizers to read. */
  normalized: Partial<Record<AssetId, unknown[]>>;
}

interface NormalizerEntry {
  normalize: (raw: any[], ctx: LoadCtx) => any[];
  /**
   * Asset ids whose normalized output this normalizer reads from
   * `ctx.normalized`. Entries with a dependency run in a second pass.
   */
  dependsOn?: AssetId[];
}

export const NORMALIZERS: Partial<Record<AssetId, NormalizerEntry>> = {
  abilityTemplates: {
    normalize: (raw, ctx) =>
      sanitizeAbilityTemplates(raw, ctx.animationMap, ctx.soundMap),
  },
  spellTemplates: {
    normalize: (raw) => sanitizeSpellTemplates(raw),
  },
  itemTemplates: {
    // sanitizeItemTemplates cross-references sanitized ability attacks/restores.
    normalize: (raw, ctx) =>
      sanitizeItemTemplates(raw, (ctx.normalized.abilityTemplates ?? []) as any),
    dependsOn: ['abilityTemplates'],
  },
  maps: {
    normalize: (raw) => normalizeMapItemsOnLoad(raw.map((m) => normalizeMapOnLoad(m))),
  },
  mapGrids: {
    normalize: (raw) => sanitizeMapGridTemplates(raw as any),
  },
};

/**
 * Normalize every raw payload. Two passes: independent normalizers (and raw
 * passthrough for types with none) first, then normalizers that depend on
 * another type's normalized output.
 */
export function normalizeAll(
  rawByType: Partial<Record<AssetId, any[]>>,
  base: Omit<LoadCtx, 'normalized'>,
  order: readonly { id: AssetId }[],
): Record<AssetId, any[]> {
  const normalized: Partial<Record<AssetId, any[]>> = {};
  const ctx: LoadCtx = { ...base, normalized };

  for (const { id } of order) {
    const entry = NORMALIZERS[id];
    normalized[id] =
      entry && !entry.dependsOn?.length
        ? entry.normalize(rawByType[id] ?? [], ctx)
        : (rawByType[id] ?? []);
  }
  for (const { id } of order) {
    const entry = NORMALIZERS[id];
    if (entry?.dependsOn?.length) {
      normalized[id] = entry.normalize(rawByType[id] ?? [], ctx);
    }
  }

  return normalized as Record<AssetId, any[]>;
}
