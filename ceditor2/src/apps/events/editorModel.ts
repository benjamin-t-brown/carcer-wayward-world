import type {
  EventChoice,
  EventChoiceNode,
  EventEndNode,
  EventExecNode,
  EventNode,
  EventSwitchNode,
  EventSwitchCase,
  KnownEventNodeType,
  SpecialEventRecord,
} from '../../core/domain/events/index.js';

const DEEP_LINK_KEYS = ['event', 'specialEvent', 'selected'] as const;

export interface EventClipboard {
  readonly anchorX: number;
  readonly anchorY: number;
  readonly nodes: readonly EventNode[];
}

export function matchesEventSearch(
  event: SpecialEventRecord,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  return (
    term === '' ||
    event.id.toLocaleLowerCase().includes(term) ||
    event.title.toLocaleLowerCase().includes(term) ||
    event.eventType.toLocaleLowerCase().includes(term)
  );
}

export function eventIdFromUrl(url: URL): string | null {
  for (const key of DEEP_LINK_KEYS) {
    const value = url.searchParams.get(key)?.trim();
    if (value) return value;
  }
  const hash = url.hash.startsWith('#') ? url.hash.slice(1) : url.hash;
  return hash && !hash.includes('=') && !hash.includes('?')
    ? decodeURIComponent(hash)
    : null;
}

export function findDeepLinkedEvent(
  events: readonly SpecialEventRecord[],
  url: URL,
): number {
  const id = eventIdFromUrl(url);
  return id ? events.findIndex((event) => event.id === id) : -1;
}

export function withEventSelection(url: URL, id?: string): URL {
  const next = new URL(url);
  for (const key of DEEP_LINK_KEYS) next.searchParams.delete(key);
  if (id?.trim()) next.searchParams.set('event', id.trim());
  return next;
}

export function createUniqueEventId(
  preferred: string,
  existingIds: Iterable<string>,
): string {
  const existing = new Set(existingIds);
  if (!existing.has(preferred)) return preferred;
  let suffix = 2;
  while (existing.has(`${preferred}_${suffix}`)) suffix += 1;
  return `${preferred}_${suffix}`;
}

export function createSpecialEvent(
  id: string,
  title = 'New Event',
): SpecialEventRecord {
  return {
    id,
    title,
    eventType: 'TALK',
    icon: '',
    vars: [],
    children: [createEventNode('EXEC', 'root', 40, 40)],
  };
}

export function cloneSpecialEvent(
  source: SpecialEventRecord,
  id: string,
): SpecialEventRecord {
  return { ...structuredClone(source), id, title: `${source.title} (copy)` };
}

export function randomNodeId(existingIds: Iterable<string>): string {
  const existing = new Set(existingIds);
  for (;;) {
    const candidate = Math.random().toString(36).slice(2, 13);
    if (candidate && !existing.has(candidate)) return candidate;
  }
}

export function createEventNode(
  type: KnownEventNodeType,
  id: string,
  x: number,
  y: number,
): EventNode {
  const base = { id, x, y, h: 72, eventChildType: type } as const;
  if (type === 'EXEC') {
    return { ...base, p: '', execStr: '', next: '', autoAdvance: false };
  }
  if (type === 'CHOICE') {
    return { ...base, h: 108, text: '', choices: [] };
  }
  if (type === 'SWITCH') {
    return { ...base, h: 108, cases: [], defaultNext: '' };
  }
  if (type === 'END') return { ...base, next: '' };
  if (type === 'COMMENT') return { ...base, comment: '' };
  return { ...base, keywords: {} };
}

export function copyEventNodes(
  nodes: readonly Readonly<EventNode>[],
  selectedIndexes: ReadonlySet<number>,
): EventClipboard | undefined {
  const selected = [...selectedIndexes]
    .sort((left, right) => left - right)
    .map((index) => nodes[index])
    .filter((node): node is Readonly<EventNode> => node !== undefined)
    .map((node) => structuredClone(node) as EventNode);
  if (selected.length === 0) return undefined;
  return {
    anchorX: Math.min(...selected.map((node) => node.x ?? 0)),
    anchorY: Math.min(...selected.map((node) => node.y ?? 0)),
    nodes: selected,
  };
}

function rewriteTargets(
  node: EventNode,
  ids: ReadonlyMap<string, string>,
): void {
  const replace = (value: unknown): unknown =>
    typeof value === 'string' ? (ids.get(value) ?? value) : value;
  if (node.eventChildType === 'EXEC' || node.eventChildType === 'END') {
    node.next = replace(node.next) as string | undefined;
  } else if (node.eventChildType === 'SWITCH') {
    node.defaultNext = replace(node.defaultNext) as string | undefined;
    const cases = Array.isArray(node.cases)
      ? (node.cases as EventSwitchCase[])
      : [];
    cases.forEach((branch) => {
      branch.next = replace(branch.next) as string | undefined;
    });
  } else if (node.eventChildType === 'CHOICE') {
    const choices = Array.isArray(node.choices)
      ? (node.choices as EventChoice[])
      : [];
    choices.forEach((branch) => {
      branch.next = replace(branch.next) as string | undefined;
    });
  }
}

/** Remove nodes and unlink every retained parent exit that targeted them. */
export function removeEventNodes(
  nodes: readonly Readonly<EventNode>[],
  selectedIndexes: ReadonlySet<number>,
): EventNode[] {
  const removedIds = new Set(
    [...selectedIndexes]
      .map((index) => nodes[index]?.id)
      .filter((id): id is string => id !== undefined),
  );
  const clear = (target: string | undefined): string | undefined =>
    target !== undefined && removedIds.has(target) ? '' : target;
  return nodes
    .filter((_, index) => !selectedIndexes.has(index))
    .map((source) => {
      const node = structuredClone(source) as EventNode;
      if (node.eventChildType === 'EXEC') {
        const typed = node as EventExecNode;
        typed.next = clear(typed.next);
      } else if (node.eventChildType === 'END') {
        const typed = node as EventEndNode;
        typed.next = clear(typed.next);
      } else if (node.eventChildType === 'SWITCH') {
        const typed = node as EventSwitchNode;
        typed.defaultNext = clear(typed.defaultNext);
        typed.cases?.forEach((branch) => {
          branch.next = clear(branch.next);
        });
      } else if (node.eventChildType === 'CHOICE') {
        (node as EventChoiceNode).choices?.forEach((branch) => {
          branch.next = clear(branch.next);
        });
      }
      return node;
    });
}

export function pasteEventNodes(
  clipboard: EventClipboard,
  existingIds: Iterable<string>,
  atX: number,
  atY: number,
): EventNode[] {
  const occupied = new Set(existingIds);
  const replacementIds = new Map<string, string>();
  for (const node of clipboard.nodes) {
    const nextId = createUniqueEventId(`${node.id}_copy`, occupied);
    occupied.add(nextId);
    replacementIds.set(node.id, nextId);
  }
  return clipboard.nodes.map((source) => {
    const node = structuredClone(source);
    node.id = replacementIds.get(source.id)!;
    node.x = atX + (source.x ?? 0) - clipboard.anchorX;
    node.y = atY + (source.y ?? 0) - clipboard.anchorY;
    rewriteTargets(node, replacementIds);
    return node;
  });
}
