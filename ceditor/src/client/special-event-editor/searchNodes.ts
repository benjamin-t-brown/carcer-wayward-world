import { Choice, SENode } from '../types/assets';
import { EditorNode } from './cmpts/EditorNode';
import { getEditorState } from './seEditorState';

const SKIP_KEYS = new Set(['x', 'y', 'h', 'autoAdvance']);
const SNIPPET_RADIUS = 36;
const LOW_PRIORITY_FIELDS = new Set(['id', 'next', 'eventChildType']);

export interface NodeSearchHit {
  nodeId: string;
  nodeType: string;
  field: string;
  snippetBefore: string;
  snippetMatch: string;
  snippetAfter: string;
  score: number;
}

function pushField(
  fields: Array<{ field: string; text: string }>,
  field: string,
  text: string | undefined,
) {
  if (typeof text === 'string' && text.trim() !== '') {
    fields.push({ field, text });
  }
}

export function collectNodeFields(
  node: object,
): Array<{ field: string; text: string }> {
  const fields: Array<{ field: string; text: string }> = [];

  const walk = (value: unknown, path: string) => {
    if (value == null) {
      return;
    }
    if (typeof value === 'string') {
      pushField(fields, path || 'value', value);
      return;
    }
    if (typeof value === 'number' || typeof value === 'boolean') {
      return;
    }
    if (Array.isArray(value)) {
      value.forEach((item, index) => {
        walk(item, `${path}[${index}]`);
      });
      return;
    }
    if (typeof value === 'object') {
      for (const [key, child] of Object.entries(
        value as Record<string, unknown>,
      )) {
        if (SKIP_KEYS.has(key)) {
          continue;
        }
        walk(child, path ? `${path}.${key}` : key);
      }
    }
  };

  walk(node, '');
  return fields;
}

function collectChoiceFields(
  choices: Choice[],
): Array<{ field: string; text: string }> {
  const fields: Array<{ field: string; text: string }> = [];
  choices.forEach((choice, index) => {
    const prefix = `choices[${index}]`;
    pushField(fields, `${prefix}.text`, choice.text);
    pushField(fields, `${prefix}.conditionStr`, choice.conditionStr);
    pushField(fields, `${prefix}.evalStr`, choice.evalStr);
    pushField(fields, `${prefix}.prefixText`, choice.prefixText);
    pushField(fields, `${prefix}.next`, choice.next);
    (choice.switchText ?? []).forEach((switchText, switchIndex) => {
      pushField(
        fields,
        `${prefix}.switchText[${switchIndex}].conditionStr`,
        switchText.conditionStr,
      );
      pushField(
        fields,
        `${prefix}.switchText[${switchIndex}].text`,
        switchText.text,
      );
    });
  });
  return fields;
}

function collectEditorNodeFields(
  editorNode: EditorNode,
): Array<{ field: string; text: string }> {
  const fields = collectNodeFields(editorNode.toSENode());
  const liveChoices = (editorNode as EditorNode & { choices?: Choice[] })
    .choices;
  if (Array.isArray(liveChoices)) {
    fields.push(...collectChoiceFields(liveChoices));
  }
  return fields;
}

export function exactMatch(
  haystack: string,
  query: string,
): { score: number; start: number; end: number } | null {
  const q = query.trim().toLowerCase();
  if (!q) {
    return null;
  }
  const start = haystack.toLowerCase().indexOf(q);
  if (start < 0) {
    return null;
  }
  return {
    score: 10000 - start - Math.abs(haystack.length - q.length),
    start,
    end: start + q.length,
  };
}

export function buildSnippet(
  text: string,
  start: number,
  end: number,
  radius = SNIPPET_RADIUS,
): { before: string; match: string; after: string } {
  const from = Math.max(0, start - radius);
  const to = Math.min(text.length, end + radius);
  const collapse = (value: string) => value.replace(/\s+/g, ' ');
  const before =
    (from > 0 ? '…' : '') + collapse(text.slice(from, start));
  let match = collapse(text.slice(start, end));
  if (match.length > 80) {
    match = `${match.slice(0, 77)}…`;
  }
  const after = collapse(text.slice(end, to)) + (to < text.length ? '…' : '');
  return { before, match, after };
}

function fieldScore(field: string, matchScore: number): number {
  if (LOW_PRIORITY_FIELDS.has(field) || field.endsWith('.next')) {
    return matchScore - 5000;
  }
  if (field.includes('evalStr') || field.includes('execStr')) {
    return matchScore + 250;
  }
  return matchScore;
}

export function searchEditorNodes(query: string): NodeSearchHit[] {
  const trimmed = query.trim();
  if (!trimmed) {
    return [];
  }

  const hits: NodeSearchHit[] = [];
  const seen = new Set<string>();
  for (const editorNode of getEditorState().editorNodes) {
    const node = editorNode.toSENode() as SENode;
    const fields = collectEditorNodeFields(editorNode);
    for (const field of fields) {
      const match = exactMatch(field.text, trimmed);
      if (!match) {
        continue;
      }
      const dedupeKey = `${node.id}:${field.field}:${match.start}:${field.text}`;
      if (seen.has(dedupeKey)) {
        continue;
      }
      seen.add(dedupeKey);
      const snippet = buildSnippet(field.text, match.start, match.end);
      hits.push({
        nodeId: node.id,
        nodeType: node.eventChildType,
        field: field.field,
        snippetBefore: snippet.before,
        snippetMatch: snippet.match,
        snippetAfter: snippet.after,
        score: fieldScore(field.field, match.score),
      });
    }
  }

  hits.sort((a, b) => {
    if (b.score !== a.score) {
      return b.score - a.score;
    }
    if (a.nodeId !== b.nodeId) {
      return a.nodeId.localeCompare(b.nodeId);
    }
    return a.field.localeCompare(b.field);
  });
  return hits;
}
