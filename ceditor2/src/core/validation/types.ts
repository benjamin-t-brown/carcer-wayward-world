import type { AssetId } from '../database/assetRegistry.js';

export type ValidationSeverity = 'error' | 'warning';

export interface ValidationIssue {
  severity: ValidationSeverity;
  code: string;
  message: string;
  path: string;
  assetId?: AssetId;
  recordId?: string;
}

export interface ValidationResult {
  valid: boolean;
  issues: readonly ValidationIssue[];
  errors: readonly ValidationIssue[];
  warnings: readonly ValidationIssue[];
}
