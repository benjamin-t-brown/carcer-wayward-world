const STORAGE_KEY = 'ceditor.seRunnerInitialState';
const DEFAULT_STATE_TEXT = '{\n}\n';

export function loadRunnerInitialStateText(): string {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (raw == null) {
      return DEFAULT_STATE_TEXT;
    }
    return raw;
  } catch {
    return DEFAULT_STATE_TEXT;
  }
}

export function saveRunnerInitialStateText(text: string): void {
  localStorage.setItem(STORAGE_KEY, text);
}

export function parseRunnerInitialState(text: string):
  | { ok: true; value: Record<string, unknown> }
  | { ok: false; error: string } {
  const trimmed = text.trim();
  if (!trimmed) {
    return { ok: true, value: {} };
  }
  try {
    const parsed: unknown = JSON.parse(trimmed);
    if (parsed === null || typeof parsed !== 'object' || Array.isArray(parsed)) {
      return { ok: false, error: 'State must be a JSON object' };
    }
    return { ok: true, value: parsed as Record<string, unknown> };
  } catch (error) {
    return { ok: false, error: (error as Error).message };
  }
}

export function cloneRunnerInitialState(
  value: Record<string, unknown>,
): Record<string, unknown> {
  return JSON.parse(JSON.stringify(value)) as Record<string, unknown>;
}

function setNestedValue(
  storage: Record<string, unknown>,
  key: string,
  value: unknown,
): void {
  const keys = key.split('.').map((part) => part.trim()).filter(Boolean);
  if (keys.length === 0) {
    return;
  }
  let curr: Record<string, unknown> = storage;
  for (let i = 0; i < keys.length - 1; i++) {
    const part = keys[i];
    const next = curr[part];
    if (typeof next !== 'object' || next === null || Array.isArray(next)) {
      curr[part] = {};
    }
    curr = curr[part] as Record<string, unknown>;
  }
  curr[keys[keys.length - 1]] = value;
}

export function addKeyToRunnerInitialState(
  text: string,
  rawKey: string,
): { ok: true; text: string } | { ok: false; error: string } {
  const key = rawKey.trim().replace(/^@/, '');
  if (!key) {
    return { ok: false, error: 'Enter a storage key to add' };
  }
  const parsed = parseRunnerInitialState(text);
  if (!parsed.ok) {
    return parsed;
  }
  setNestedValue(parsed.value, key, 'true');
  return { ok: true, text: `${JSON.stringify(parsed.value, null, 2)}\n` };
}
