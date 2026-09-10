import type { JsonObject } from '../../database/types.js';

export const RUNTIME_EVENT_TYPES = ['MODAL', 'TALK'] as const;
export const KNOWN_EVENT_NODE_TYPES = [
  'EXEC',
  'CHOICE',
  'SWITCH',
  'END',
  'COMMENT',
  'KEYWORD',
] as const;
export const RUNTIME_EVENT_NODE_TYPES = [
  'EXEC',
  'CHOICE',
  'SWITCH',
  'END',
] as const;

export type RuntimeEventType = (typeof RUNTIME_EVENT_TYPES)[number];
export type KnownEventNodeType = (typeof KNOWN_EVENT_NODE_TYPES)[number];
export type RuntimeEventNodeType = (typeof RUNTIME_EVENT_NODE_TYPES)[number];

export type EventVariable = JsonObject & {
  id?: string;
  key?: string;
  value?: string;
  importFrom?: string;
};

export type EventAudioInfo = JsonObject & {
  audioName?: string;
  volume?: number;
  offset?: number;
};

export type EventChoiceAlternateText = JsonObject & {
  conditionStr?: string;
  text?: string;
};

export type EventChoice = JsonObject & {
  text?: string;
  switchText?: EventChoiceAlternateText[];
  prefixText?: string;
  conditionStr?: string;
  evalStr?: string;
  next?: string;
};

export type EventSwitchCase = JsonObject & {
  conditionStr?: string;
  next?: string;
};

export type EventNodeBase = JsonObject & {
  id: string;
  /** Unknown future or legacy kinds are retained rather than coerced. */
  eventChildType: string;
  x?: number;
  y?: number;
  h?: number;
};

export type EventExecNode = EventNodeBase & {
  eventChildType: 'EXEC';
  p?: string;
  execStr?: string;
  next?: string;
  autoAdvance?: boolean;
  audioInfo?: EventAudioInfo;
};

export type EventChoiceNode = EventNodeBase & {
  eventChildType: 'CHOICE';
  text?: string;
  choices?: EventChoice[];
  audioInfo?: EventAudioInfo;
};

export type EventSwitchNode = EventNodeBase & {
  eventChildType: 'SWITCH';
  defaultNext?: string;
  cases?: EventSwitchCase[];
};

export type EventEndNode = EventNodeBase & {
  eventChildType: 'END';
  next?: string;
};

export type EventCommentNode = EventNodeBase & {
  eventChildType: 'COMMENT';
  comment?: string;
};

export type EventKeywordNode = EventNodeBase & {
  eventChildType: 'KEYWORD';
  keywords?: JsonObject;
};

export type UnknownEventNode = EventNodeBase;

/**
 * The raw, lossless graph node. Known shapes are available through type guards;
 * the base member keeps unknown node kinds representable.
 */
export type EventNode =
  | EventExecNode
  | EventChoiceNode
  | EventSwitchNode
  | EventEndNode
  | EventCommentNode
  | EventKeywordNode
  | UnknownEventNode;

export type SpecialEventRecord = JsonObject & {
  id: string;
  title: string;
  /** Unknown values are preserved so validation can report them without loss. */
  eventType: string;
  icon: string;
  vars?: EventVariable[];
  children?: EventNode[];
};

export type EventConnectionKind =
  'next' | 'defaultNext' | 'switchCase' | 'choice';

export interface EventConnection {
  readonly sourceId: string;
  readonly targetId: string;
  readonly kind: EventConnectionKind;
  readonly branchIndex?: number;
}

export type EventGraphIssueCode =
  | 'duplicate-node-id'
  | 'missing-target'
  | 'missing-root'
  | 'unreachable-node'
  | 'unknown-node-type'
  | 'runtime-unsupported-node-type';

export interface EventGraphIssue {
  readonly severity: 'error' | 'warning';
  readonly code: EventGraphIssueCode;
  readonly message: string;
  readonly nodeId?: string;
  readonly targetId?: string;
}

export interface EventGraphAnalysis {
  readonly nodesById: ReadonlyMap<string, readonly EventNode[]>;
  readonly connections: readonly EventConnection[];
  readonly incomingByNodeId: ReadonlyMap<string, readonly EventConnection[]>;
  readonly reachableNodeIds: ReadonlySet<string>;
  readonly issues: readonly EventGraphIssue[];
}
