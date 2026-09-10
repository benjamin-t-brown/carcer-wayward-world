import {
  parseEventNode,
  parseEventVariables,
  parseSpecialEventRecord,
} from './eventParser.js';
import { analyzeEventGraph } from './eventGraph.js';
import type {
  EventGraphAnalysis,
  EventNode,
  EventVariable,
  SpecialEventRecord,
} from './types.js';

export interface SpecialEventHeaderPatch {
  readonly id?: string;
  readonly title?: string;
  readonly eventType?: string;
  readonly icon?: string;
}

/**
 * Mutable event document with explicit, index-based edits. Indexes remain safe
 * even while a malformed legacy graph contains duplicate IDs.
 */
export class EventDocument {
  readonly #record: SpecialEventRecord;

  private constructor(record: SpecialEventRecord) {
    this.#record = record;
  }

  static from(value: unknown, path = 'specialEvent'): EventDocument {
    return new EventDocument(parseSpecialEventRecord(value, path));
  }

  get id(): string {
    return this.#record.id;
  }

  get title(): string {
    return this.#record.title;
  }

  get eventType(): string {
    return this.#record.eventType;
  }

  get icon(): string {
    return this.#record.icon;
  }

  get nodeCount(): number {
    return this.#record.children?.length ?? 0;
  }

  /** Read-only hot-path view for rendering; mutations go through this class. */
  get nodes(): readonly Readonly<EventNode>[] {
    return this.#record.children ?? [];
  }

  get variables(): readonly Readonly<EventVariable>[] {
    return this.#record.vars ?? [];
  }

  nodeAt(index: number): Readonly<EventNode> | undefined {
    return Number.isSafeInteger(index) && index >= 0
      ? this.#record.children?.[index]
      : undefined;
  }

  nodeIndexes(id: string): readonly number[] {
    const indexes: number[] = [];
    this.#record.children?.forEach((node, index) => {
      if (node.id === id) indexes.push(index);
    });
    return indexes;
  }

  updateHeader(patch: SpecialEventHeaderPatch): void {
    for (const [key, value] of Object.entries(patch)) {
      if (typeof value !== 'string') {
        throw new TypeError(`specialEvent.${key}: expected a string`);
      }
      this.#record[key] = value;
    }
  }

  replaceVariables(value: unknown): void {
    this.#record.vars = parseEventVariables(value);
  }

  insertNode(value: unknown, index = this.nodeCount): void {
    if (!Number.isSafeInteger(index) || index < 0 || index > this.nodeCount) {
      throw new RangeError(`event node index ${index} is out of bounds`);
    }
    const node = parseEventNode(value, `specialEvent.children[${index}]`);
    const children = (this.#record.children ??= []);
    children.splice(index, 0, node);
  }

  replaceNode(index: number, value: unknown): void {
    const children = this.#requiredChildrenIndex(index);
    children[index] = parseEventNode(value, `specialEvent.children[${index}]`);
  }

  patchNode(index: number, patch: Readonly<Record<string, unknown>>): void {
    const children = this.#requiredChildrenIndex(index);
    const next = { ...children[index], ...structuredClone(patch) };
    children[index] = parseEventNode(next, `specialEvent.children[${index}]`);
  }

  setNodePosition(index: number, x: number, y: number): void {
    if (!Number.isFinite(x) || !Number.isFinite(y)) {
      throw new TypeError('event node position must be finite');
    }
    this.patchNode(index, { x, y });
  }

  /**
   * Removes exactly one array element and returns its detached value. Incoming
   * links are deliberately not changed; callers can preview them via analyze().
   */
  removeNode(index: number): EventNode {
    const children = this.#requiredChildrenIndex(index);
    return structuredClone(children.splice(index, 1)[0]!);
  }

  analyze(): EventGraphAnalysis {
    return analyzeEventGraph(this.#record);
  }

  snapshot(): SpecialEventRecord {
    return structuredClone(this.#record);
  }

  #requiredChildrenIndex(index: number): EventNode[] {
    const children = this.#record.children;
    if (
      !children ||
      !Number.isSafeInteger(index) ||
      index < 0 ||
      index >= children.length
    ) {
      throw new RangeError(`event node index ${index} is out of bounds`);
    }
    return children;
  }
}
