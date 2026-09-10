import type { JsonArray } from '../../core/database/index.js';
import {
  EventDocument,
  parseSpecialEventCollection,
  RUNTIME_EVENT_TYPES,
  type EventNode,
  type KnownEventNodeType,
  type SpecialEventRecord,
} from '../../core/domain/events/index.js';
import { element } from '../../core/ui/dom.js';
import type { DatabasePageContext } from '../../core/ui/index.js';
import {
  cloneSpecialEvent,
  copyEventNodes,
  createEventNode,
  createSpecialEvent,
  createUniqueEventId,
  findDeepLinkedEvent,
  matchesEventSearch,
  pasteEventNodes,
  randomNodeId,
  removeEventNodes,
  withEventSelection,
  type EventClipboard,
} from './editorModel.js';
import { EventRenderer } from './EventRenderer.js';
import {
  fitEventViewport,
  hitTestEventNode,
  indexesInWorldRectangle,
  screenToWorld,
  type EventViewport,
  type Point,
} from './graphGeometry.js';
import {
  deleteEventAcrossDatabase,
  previewEventReferences,
  renameEventAcrossDatabase,
  type EventDatabaseRecords,
  type EventLifecycleResult,
  type EventReferencePreview,
} from './eventLifecycle.js';
import { createEventEditorView, type EventEditorElements } from './view.js';

type DragState =
  | { kind: 'pan'; screen: Point; initial: EventViewport }
  | {
      kind: 'nodes';
      world: Point;
      initial: ReadonlyMap<number, Point>;
      moved: boolean;
    }
  | { kind: 'select'; world: Point; additive: boolean };

function button(text: string, onClick: () => void, className = 'button') {
  const result = element('button', {
    text,
    className,
    attributes: { type: 'button' },
  });
  result.addEventListener('click', onClick);
  return result;
}

function labeled(
  labelText: string,
  control: HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement,
): HTMLDivElement {
  const wrapper = element('div', { className: 'field' });
  const id = `event-field-${Math.random().toString(36).slice(2)}`;
  control.id = id;
  wrapper.append(
    element('label', { text: labelText, attributes: { for: id } }),
    control,
  );
  return wrapper;
}

function textInput(value: string): HTMLInputElement {
  const input = element('input');
  input.value = value;
  return input;
}

function textArea(value: string, rows = 4): HTMLTextAreaElement {
  const textarea = element('textarea');
  textarea.value = value;
  textarea.rows = rows;
  return textarea;
}

function fieldset(title: string): HTMLFieldSetElement {
  const field = element('fieldset', { className: 'form-section' });
  field.append(element('legend', { text: title }));
  return field;
}

function selectionStatus(count: number, total: number): string {
  return count === 0
    ? `${total} nodes · drag empty space to select · Alt/middle-drag to pan`
    : `${count} of ${total} nodes selected · drag to move · Ctrl/Cmd+C and V to duplicate`;
}

export class EventEditorController {
  private readonly view: EventEditorElements;
  private readonly renderer = new EventRenderer();
  private records: SpecialEventRecord[];
  private selectedEventIndex: number;
  private document?: EventDocument;
  private searchTerm = '';
  private selectedNodes = new Set<number>();
  private viewport: EventViewport = { x: 48, y: 48, scale: 1 };
  private readonly viewportsByEventId = new Map<string, EventViewport>();
  private lastPointerWorld?: Point;
  private drag?: DragState;
  private selectionEnd?: Point;
  private clipboard?: EventClipboard;
  private animationFrame = 0;
  private destroyed = false;

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseSpecialEventCollection(
      context.session.collection('specialEvents'),
    );
    this.selectedEventIndex = findDeepLinkedEvent(this.records, initialUrl);
    if (this.selectedEventIndex < 0 && this.records.length > 0) {
      const recent = this.readRecent().find((id) =>
        this.records.some((event) => event.id === id),
      );
      this.selectedEventIndex = recent
        ? this.records.findIndex((event) => event.id === recent)
        : 0;
    }
    this.loadDocument();
    this.view = createEventEditorView();
  }

  mount(): void {
    this.root.replaceChildren(this.view.layout);
    this.bindView();
    this.renderList();
    this.renderInspector();
    this.fitGraph();
    this.animationFrame = requestAnimationFrame(this.draw);
  }

  destroy(): void {
    this.destroyed = true;
    cancelAnimationFrame(this.animationFrame);
    window.removeEventListener('pointermove', this.pointerMove);
    window.removeEventListener('pointerup', this.pointerUp);
    this.root.replaceChildren();
  }

  private bindView(): void {
    this.view.search.addEventListener('input', () => {
      this.searchTerm = this.view.search.value;
      this.renderList();
    });
    this.view.newButton.addEventListener('click', () => this.createEvent());
    this.view.fitButton.addEventListener('click', () => this.fitGraph());
    this.view.findNodeButton.addEventListener('click', () =>
      this.findAndCenterNode(),
    );
    this.view.findNodeInput.addEventListener('keydown', (event) => {
      if (event.key === 'Enter') this.findAndCenterNode();
    });
    for (const [type, addButton] of this.view.addButtons) {
      addButton.addEventListener('click', () =>
        this.addNode(type as KnownEventNodeType),
      );
    }
    this.view.deleteNodesButton.addEventListener(
      'click',
      () => void this.deleteSelectedNodes(),
    );
    this.view.canvas.addEventListener('contextmenu', (event) =>
      event.preventDefault(),
    );
    this.view.canvas.addEventListener('pointerdown', this.pointerDown);
    window.addEventListener('pointermove', this.pointerMove);
    window.addEventListener('pointerup', this.pointerUp);
    this.view.canvas.addEventListener('wheel', this.wheel, { passive: false });
    this.view.canvas.addEventListener('keydown', this.keyDown);
  }

  private selectedEvent(): SpecialEventRecord | undefined {
    return this.records[this.selectedEventIndex];
  }

  private loadDocument(): void {
    const event = this.selectedEvent();
    this.document = event ? EventDocument.from(event) : undefined;
    this.selectedNodes.clear();
  }

  private commitDocument(renderInspector = true): void {
    if (!this.document || this.selectedEventIndex < 0) return;
    this.records[this.selectedEventIndex] = this.document.snapshot();
    this.context.session.replaceCollection(
      'specialEvents',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
    const event = this.selectedEvent();
    if (event) {
      window.history.replaceState(
        null,
        '',
        withEventSelection(new URL(window.location.href), event.id),
      );
      this.writeRecent(event.id);
    }
    this.renderList();
    if (renderInspector) this.renderInspector();
  }

  private setEventSelection(index: number): void {
    if (index < 0 || index >= this.records.length) return;
    const currentId = this.selectedEvent()?.id;
    if (currentId) this.viewportsByEventId.set(currentId, { ...this.viewport });
    this.selectedEventIndex = index;
    this.loadDocument();
    const event = this.selectedEvent()!;
    window.history.replaceState(
      null,
      '',
      withEventSelection(new URL(window.location.href), event.id),
    );
    this.writeRecent(event.id);
    this.renderList();
    this.renderInspector();
    const retainedViewport = this.viewportsByEventId.get(event.id);
    if (retainedViewport) {
      this.viewport = { ...retainedViewport };
    } else {
      this.fitGraph();
    }
  }

  private renderList(): void {
    this.view.list.replaceChildren();
    const matches = this.records
      .map((event, index) => ({ event, index }))
      .filter(({ event }) => matchesEventSearch(event, this.searchTerm));
    this.view.resultCount.textContent = `${matches.length} of ${this.records.length} events`;
    for (const { event, index } of matches) {
      const item = element('li');
      const select = element('button', {
        className: 'event-card',
        attributes: {
          type: 'button',
          'aria-current': String(index === this.selectedEventIndex),
        },
      });
      select.append(
        element('strong', { text: event.title || '(untitled)' }),
        element('span', {
          className: 'event-card__meta',
          text: `${event.id} · ${event.eventType}`,
        }),
      );
      select.addEventListener('click', () => this.setEventSelection(index));
      item.append(select);
      this.view.list.append(item);
    }
  }

  private renderInspector(): void {
    this.view.inspector.replaceChildren();
    const event = this.selectedEvent();
    if (!event || !this.document) {
      this.view.inspector.append(element('p', { text: 'No event selected.' }));
      return;
    }
    const heading = element('div', { className: 'event-inspector__heading' });
    heading.append(
      element('h2', { text: event.title || event.id }),
      button('Clone', () => this.cloneEvent(), 'button button--small'),
      button(
        'Delete event',
        () => this.deleteEvent(),
        'button button--small button--danger',
      ),
    );
    this.view.inspector.append(heading, this.headerFields());
    if (this.selectedNodes.size === 1) {
      const index = [...this.selectedNodes][0]!;
      const node = this.document.nodeAt(index);
      if (node) this.view.inspector.append(this.nodeFields(index, node));
    } else if (this.selectedNodes.size > 1) {
      const multi = fieldset('Selected nodes');
      multi.append(
        element('p', {
          text: `${this.selectedNodes.size} nodes selected. Drag any selected node to move the group, copy/paste it, or delete it.`,
        }),
      );
      this.view.inspector.append(multi);
    }
    this.view.inspector.append(
      this.variablesFields(),
      this.diagnosticsFields(),
      this.rawPreview(),
    );
    this.updateCanvasStatus();
  }

  private headerFields(): HTMLFieldSetElement {
    const event = this.selectedEvent()!;
    const result = fieldset('Event');
    result.classList.add('form-stack');
    const id = textInput(event.id);
    id.addEventListener('change', () => void this.renameEvent(id.value));
    const title = textInput(event.title);
    title.addEventListener('change', () => {
      this.document!.updateHeader({ title: title.value });
      this.commitDocument();
    });
    const type = element('select');
    const types = [...RUNTIME_EVENT_TYPES];
    if (!types.includes(event.eventType as never))
      types.unshift(event.eventType as never);
    for (const value of types) {
      const option = element('option', { text: value });
      option.value = value;
      type.append(option);
    }
    type.value = event.eventType;
    type.addEventListener('change', () => {
      this.document!.updateHeader({ eventType: type.value });
      this.commitDocument();
    });
    const icon = textInput(event.icon);
    icon.addEventListener('change', () => {
      this.document!.updateHeader({ icon: icon.value });
      this.commitDocument();
    });
    result.append(
      labeled('API ID', id),
      labeled('Title', title),
      labeled('Runtime type', type),
      labeled('Icon sprite', icon),
    );
    return result;
  }

  private variablesFields(): HTMLFieldSetElement {
    const result = fieldset('Variables and imports');
    const editor = textArea(
      JSON.stringify(this.document!.variables, null, 2),
      8,
    );
    const error = element('p', { className: 'field__error' });
    const apply = button(
      'Apply variables JSON',
      () => {
        try {
          this.document!.replaceVariables(JSON.parse(editor.value) as unknown);
          error.textContent = '';
          this.commitDocument();
        } catch (caught) {
          error.textContent =
            caught instanceof Error ? caught.message : String(caught);
        }
      },
      'button button--small',
    );
    result.append(
      element('p', {
        className: 'form-section__description',
        text: 'Each entry may define id, key/value, or importFrom. Unknown fields are retained.',
      }),
      labeled('Variables JSON', editor),
      error,
      apply,
    );
    return result;
  }

  private nodeFields(
    index: number,
    node: Readonly<EventNode>,
  ): HTMLFieldSetElement {
    const result = fieldset(`${node.eventChildType} node`);
    result.classList.add('form-stack');
    const id = textInput(node.id);
    id.addEventListener('change', () =>
      this.patchNode(index, { id: id.value }),
    );
    result.append(
      labeled('Node ID', id),
      button(
        'Copy node ID',
        () => void navigator.clipboard.writeText(node.id),
        'button button--small',
      ),
    );
    if (node.eventChildType === 'EXEC') {
      result.append(
        this.nodeTextField(index, 'Text / p', 'p', String(node.p ?? ''), 4),
        this.nodeTextField(
          index,
          'Expression / execStr',
          'execStr',
          String(node.execStr ?? ''),
          3,
        ),
        this.nodeTextField(
          index,
          'Next node ID',
          'next',
          String(node.next ?? ''),
        ),
        this.nodeJsonField(
          index,
          'Audio info',
          'audioInfo',
          typeof node.audioInfo === 'object' && node.audioInfo !== null
            ? node.audioInfo
            : {},
        ),
      );
      const autoAdvance = element('input');
      autoAdvance.type = 'checkbox';
      autoAdvance.checked = node.autoAdvance === true;
      autoAdvance.addEventListener('change', () =>
        this.patchNode(index, { autoAdvance: autoAdvance.checked }),
      );
      result.append(labeled('Auto advance', autoAdvance));
    } else if (node.eventChildType === 'CHOICE') {
      result.append(
        this.nodeTextField(
          index,
          'Intro text',
          'text',
          String(node.text ?? ''),
          3,
        ),
        this.nodeJsonField(
          index,
          'Choices and branch links',
          'choices',
          node.choices ?? [],
        ),
        this.nodeJsonField(
          index,
          'Audio info',
          'audioInfo',
          typeof node.audioInfo === 'object' && node.audioInfo !== null
            ? node.audioInfo
            : {},
        ),
      );
    } else if (node.eventChildType === 'SWITCH') {
      result.append(
        this.nodeTextField(
          index,
          'Default next node ID',
          'defaultNext',
          String(node.defaultNext ?? ''),
        ),
        this.nodeJsonField(
          index,
          'Cases and branch links',
          'cases',
          node.cases ?? [],
        ),
      );
    } else if (node.eventChildType === 'END') {
      result.append(
        this.nodeTextField(
          index,
          'Next node ID',
          'next',
          String(node.next ?? ''),
        ),
      );
    } else if (node.eventChildType === 'COMMENT') {
      result.append(
        this.nodeTextField(
          index,
          'Comment',
          'comment',
          String(node.comment ?? ''),
          5,
        ),
      );
    } else if (node.eventChildType === 'KEYWORD') {
      result.append(
        this.nodeJsonField(index, 'Keywords', 'keywords', node.keywords ?? {}),
      );
    }
    result.append(this.rawNodeEditor(index, node));
    return result;
  }

  private nodeTextField(
    index: number,
    label: string,
    key: string,
    value: string,
    rows?: number,
  ): HTMLDivElement {
    const control = rows ? textArea(value, rows) : textInput(value);
    control.addEventListener('change', () =>
      this.patchNode(index, { [key]: control.value }),
    );
    return labeled(label, control);
  }

  private nodeJsonField(
    index: number,
    label: string,
    key: string,
    value: unknown,
  ): HTMLDivElement {
    const wrapper = element('div', { className: 'event-json-field' });
    const editor = textArea(JSON.stringify(value, null, 2), 7);
    const error = element('p', { className: 'field__error' });
    wrapper.append(
      labeled(label, editor),
      error,
      button(
        'Apply JSON',
        () => {
          try {
            this.patchNode(index, {
              [key]: JSON.parse(editor.value) as unknown,
            });
            error.textContent = '';
          } catch (caught) {
            error.textContent =
              caught instanceof Error ? caught.message : String(caught);
          }
        },
        'button button--small',
      ),
    );
    return wrapper;
  }

  private rawNodeEditor(
    index: number,
    node: Readonly<EventNode>,
  ): HTMLDetailsElement {
    const details = element('details');
    details.append(element('summary', { text: 'Raw node JSON' }));
    const editor = textArea(JSON.stringify(node, null, 2), 10);
    const error = element('p', { className: 'field__error' });
    details.append(
      editor,
      error,
      button(
        'Replace node from JSON',
        () => {
          try {
            this.document!.replaceNode(
              index,
              JSON.parse(editor.value) as unknown,
            );
            this.commitDocument();
          } catch (caught) {
            error.textContent =
              caught instanceof Error ? caught.message : String(caught);
          }
        },
        'button button--small',
      ),
    );
    return details;
  }

  private patchNode(
    index: number,
    patch: Readonly<Record<string, unknown>>,
  ): void {
    try {
      this.document!.patchNode(index, patch);
      this.commitDocument();
    } catch (caught) {
      window.alert(caught instanceof Error ? caught.message : String(caught));
    }
  }

  private diagnosticsFields(): HTMLFieldSetElement {
    const result = fieldset('Graph diagnostics');
    const analysis = this.document!.analyze();
    if (analysis.issues.length === 0) {
      result.append(
        element('p', { className: 'event-ok', text: 'No graph issues found.' }),
      );
    } else {
      const list = element('ul', { className: 'event-diagnostics' });
      for (const issue of analysis.issues) {
        const item = element('li', {
          className: `event-diagnostic event-diagnostic--${issue.severity}`,
        });
        if (
          issue.nodeId &&
          this.document!.nodeIndexes(issue.nodeId).length > 0
        ) {
          item.append(
            button(
              issue.message,
              () => this.selectAndCenterNode(issue.nodeId!),
              'event-diagnostic__link',
            ),
          );
        } else {
          item.textContent = issue.message;
        }
        list.append(item);
      }
      result.append(list);
    }
    return result;
  }

  private rawPreview(): HTMLDetailsElement {
    const details = element('details');
    const json = JSON.stringify(this.document!.snapshot(), null, 2);
    details.append(
      element('summary', { text: 'Raw event JSON preview' }),
      button(
        'Copy event JSON',
        () => void navigator.clipboard.writeText(json),
        'button button--small',
      ),
      element('pre', {
        className: 'event-raw-preview',
        text: json,
      }),
    );
    return details;
  }

  private createEvent(): void {
    const id = createUniqueEventId(
      'new_event',
      this.records.map((event) => event.id),
    );
    this.records.push(createSpecialEvent(id));
    this.context.session.replaceCollection(
      'specialEvents',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
    this.searchTerm = '';
    this.view.search.value = '';
    this.setEventSelection(this.records.length - 1);
  }

  private cloneEvent(): void {
    const event = this.selectedEvent();
    if (!event) return;
    const id = createUniqueEventId(
      `${event.id}_copy`,
      this.records.map((item) => item.id),
    );
    this.records.push(cloneSpecialEvent(event, id));
    this.context.session.replaceCollection(
      'specialEvents',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
    this.setEventSelection(this.records.length - 1);
  }

  private async renameEvent(nextId: string): Promise<void> {
    const event = this.selectedEvent();
    if (!event || nextId.trim() === event.id) return;
    try {
      const database = this.eventDatabaseRecords();
      const preview = previewEventReferences(database, event.id);
      const selectedPaths = await this.chooseReferences(
        'Rename event',
        `Rename "${event.id}" to "${nextId.trim()}" and update the selected references?`,
        preview,
      );
      if (!selectedPaths) {
        this.renderInspector();
        return;
      }
      this.applyLifecycleResult(
        renameEventAcrossDatabase(database, event.id, nextId, selectedPaths),
      );
      this.loadDocument();
      this.renderList();
      this.renderInspector();
    } catch (caught) {
      window.alert(caught instanceof Error ? caught.message : String(caught));
      this.renderInspector();
    }
  }

  private async deleteEvent(): Promise<void> {
    const event = this.selectedEvent();
    if (!event) return;
    try {
      const database = this.eventDatabaseRecords();
      const preview = previewEventReferences(database, event.id);
      const selectedPaths = await this.chooseReferences(
        'Delete event',
        `Delete "${event.id}"? Selected references will be removed or cleared. Unselected references will remain visible as validation errors.`,
        preview,
      );
      if (!selectedPaths) return;
      this.applyLifecycleResult(
        deleteEventAcrossDatabase(database, event.id, selectedPaths),
      );
      this.selectedEventIndex = Math.min(
        this.selectedEventIndex,
        this.records.length - 1,
      );
      this.loadDocument();
      this.renderList();
      this.renderInspector();
      this.fitGraph();
    } catch (caught) {
      window.alert(caught instanceof Error ? caught.message : String(caught));
    }
  }

  private eventDatabaseRecords(): EventDatabaseRecords {
    const snapshot = this.context.session.snapshot();
    return {
      specialEvents: snapshot.specialEvents,
      maps: snapshot.maps,
      characters: snapshot.characters,
      items: snapshot.items,
    };
  }

  private applyLifecycleResult(result: EventLifecycleResult): void {
    this.context.session.replaceCollection(
      'specialEvents',
      result.specialEvents,
    );
    this.context.session.replaceCollection('maps', result.maps);
    this.context.session.replaceCollection('characters', result.characters);
    this.context.session.replaceCollection('items', result.items);
    this.records = parseSpecialEventCollection(result.specialEvents);
    this.context.notifyChanged();
  }

  private chooseReferences(
    title: string,
    message: string,
    preview: EventReferencePreview,
  ): Promise<ReadonlySet<string> | undefined> {
    return new Promise((resolve) => {
      const dialog = element('dialog', { className: 'event-reference-dialog' });
      const form = element('form', { className: 'form-stack' });
      form.method = 'dialog';
      form.append(
        element('h2', { text: title }),
        element('p', { text: message }),
      );
      const choices = new Map<string, HTMLInputElement>();
      if (preview.references.length === 0) {
        form.append(
          element('p', {
            className: 'muted',
            text: 'No structured database references point to this event.',
          }),
        );
      } else {
        const list = element('div', { className: 'event-reference-list' });
        for (const reference of preview.references) {
          const row = element('label', { className: 'check-field' });
          const checkbox = element('input');
          checkbox.type = 'checkbox';
          checkbox.checked = true;
          choices.set(reference.path, checkbox);
          row.append(
            checkbox,
            element('span', {
              text: `${reference.label} — ${reference.path}`,
            }),
          );
          list.append(row);
        }
        form.append(list);
      }
      const actions = element('div', { className: 'button-row' });
      const cancel = element('button', {
        className: 'button',
        text: 'Cancel',
        attributes: { type: 'submit', value: 'cancel' },
      });
      const confirm = element('button', {
        className: 'button button--danger',
        text: title,
        attributes: { type: 'submit', value: 'confirm' },
      });
      actions.append(cancel, confirm);
      form.append(actions);
      dialog.append(form);
      dialog.addEventListener(
        'close',
        () => {
          const selectedPaths =
            dialog.returnValue === 'confirm'
              ? new Set(
                  [...choices]
                    .filter(([, checkbox]) => checkbox.checked)
                    .map(([path]) => path),
                )
              : undefined;
          dialog.remove();
          resolve(selectedPaths);
        },
        { once: true },
      );
      document.body.append(dialog);
      dialog.showModal();
    });
  }

  private confirmAction(title: string, message: string): Promise<boolean> {
    return new Promise((resolve) => {
      const dialog = element('dialog', { className: 'event-reference-dialog' });
      const form = element('form', { className: 'form-stack' });
      form.method = 'dialog';
      form.append(
        element('h2', { text: title }),
        element('p', { text: message }),
      );
      const actions = element('div', { className: 'button-row' });
      actions.append(
        element('button', {
          className: 'button',
          text: 'Cancel',
          attributes: { type: 'submit', value: 'cancel' },
        }),
        element('button', {
          className: 'button button--danger',
          text: title,
          attributes: { type: 'submit', value: 'confirm' },
        }),
      );
      form.append(actions);
      dialog.append(form);
      dialog.addEventListener(
        'close',
        () => {
          const confirmed = dialog.returnValue === 'confirm';
          dialog.remove();
          resolve(confirmed);
        },
        { once: true },
      );
      document.body.append(dialog);
      dialog.showModal();
    });
  }

  private addNode(type: KnownEventNodeType): void {
    if (!this.document) return;
    const center = screenToWorld(
      { x: this.view.canvas.width / 2, y: this.view.canvas.height / 2 },
      this.viewport,
    );
    const id = randomNodeId(this.document.nodes.map((node) => node.id));
    this.document.insertNode(createEventNode(type, id, center.x, center.y));
    this.selectedNodes = new Set([this.document.nodeCount - 1]);
    this.commitDocument();
  }

  private async deleteSelectedNodes(): Promise<void> {
    if (!this.document || this.selectedNodes.size === 0) return;
    const confirmed = await this.confirmAction(
      'Delete nodes',
      `Delete ${this.selectedNodes.size} selected node${this.selectedNodes.size === 1 ? '' : 's'} and unlink every parent exit that targets them?`,
    );
    if (!confirmed || !this.document) return;
    const snapshot = this.document.snapshot();
    snapshot.children = removeEventNodes(
      this.document.nodes,
      this.selectedNodes,
    );
    this.document = EventDocument.from(snapshot);
    this.selectedNodes.clear();
    this.commitDocument();
  }

  private findAndCenterNode(): void {
    this.selectAndCenterNode(this.view.findNodeInput.value.trim());
  }

  private selectAndCenterNode(id: string): void {
    if (!this.document) return;
    const index = this.document.nodeIndexes(id)[0];
    const node = index === undefined ? undefined : this.document.nodeAt(index);
    if (index === undefined || !node) {
      this.view.canvasStatus.textContent = `Node "${id}" was not found.`;
      return;
    }
    this.selectedNodes = new Set([index]);
    this.viewport.x =
      this.view.canvas.width / 2 - ((node.x ?? 0) + 110) * this.viewport.scale;
    this.viewport.y =
      this.view.canvas.height / 2 -
      ((node.y ?? 0) + Math.max(64, node.h ?? 64) / 2) * this.viewport.scale;
    this.renderInspector();
  }

  private fitGraph(): void {
    const nodes = this.document?.nodes ?? [];
    this.ensureCanvasSize();
    this.viewport = fitEventViewport(
      nodes,
      this.view.canvas.width,
      this.view.canvas.height,
    );
  }

  private canvasPoint(event: PointerEvent | WheelEvent): Point {
    const bounds = this.view.canvas.getBoundingClientRect();
    return { x: event.clientX - bounds.left, y: event.clientY - bounds.top };
  }

  private readonly pointerDown = (event: PointerEvent): void => {
    if (!this.document) return;
    this.view.canvas.focus();
    const screen = this.canvasPoint(event);
    const world = screenToWorld(screen, this.viewport);
    if (event.button === 1 || event.button === 2 || event.altKey) {
      this.drag = { kind: 'pan', screen, initial: { ...this.viewport } };
      return;
    }
    const hit = hitTestEventNode(this.document.nodes, world);
    if (hit >= 0) {
      if (event.shiftKey) {
        if (this.selectedNodes.has(hit)) {
          this.selectedNodes.delete(hit);
        } else {
          this.selectedNodes.add(hit);
        }
      } else if (!this.selectedNodes.has(hit)) {
        this.selectedNodes = new Set([hit]);
      }
      const initial = new Map<number, Point>();
      for (const index of this.selectedNodes) {
        const node = this.document.nodeAt(index);
        if (node) initial.set(index, { x: node.x ?? 0, y: node.y ?? 0 });
      }
      this.drag = { kind: 'nodes', world, initial, moved: false };
      this.renderInspector();
    } else {
      this.drag = { kind: 'select', world, additive: event.shiftKey };
      this.selectionEnd = world;
      if (!event.shiftKey) this.selectedNodes.clear();
      this.renderInspector();
    }
  };

  private readonly pointerMove = (event: PointerEvent): void => {
    if (!this.drag || !this.document) return;
    const screen = this.canvasPoint(event);
    if (this.drag.kind === 'pan') {
      this.viewport.x = this.drag.initial.x + screen.x - this.drag.screen.x;
      this.viewport.y = this.drag.initial.y + screen.y - this.drag.screen.y;
    } else {
      const world = screenToWorld(screen, this.viewport);
      if (this.drag.kind === 'select') {
        this.selectionEnd = world;
      } else {
        const dx = world.x - this.drag.world.x;
        const dy = world.y - this.drag.world.y;
        if (dx !== 0 || dy !== 0) this.drag.moved = true;
        for (const [index, origin] of this.drag.initial) {
          this.document.setNodePosition(index, origin.x + dx, origin.y + dy);
        }
      }
    }
  };

  private readonly pointerUp = (event: PointerEvent): void => {
    if (!this.drag || !this.document) return;
    if (this.drag.kind === 'select') {
      const end = screenToWorld(this.canvasPoint(event), this.viewport);
      const boxed = indexesInWorldRectangle(
        this.document.nodes,
        this.drag.world,
        end,
      );
      if (this.drag.additive) {
        for (const index of boxed) this.selectedNodes.add(index);
      } else {
        this.selectedNodes = boxed;
      }
    } else if (this.drag.kind === 'nodes' && this.drag.moved) {
      this.commitDocument(false);
    }
    this.drag = undefined;
    this.selectionEnd = undefined;
    this.renderInspector();
  };

  private readonly wheel = (event: WheelEvent): void => {
    event.preventDefault();
    const screen = this.canvasPoint(event);
    const world = screenToWorld(screen, this.viewport);
    const scale = Math.min(
      3,
      Math.max(0.12, this.viewport.scale * Math.exp(-event.deltaY * 0.001)),
    );
    this.viewport = {
      scale,
      x: screen.x - world.x * scale,
      y: screen.y - world.y * scale,
    };
  };

  private readonly keyDown = (event: KeyboardEvent): void => {
    if (!this.document || !(event.metaKey || event.ctrlKey)) return;
    if (event.key.toLocaleLowerCase() === 'c') {
      this.clipboard = copyEventNodes(this.document.nodes, this.selectedNodes);
      if (this.clipboard) event.preventDefault();
    } else if (event.key.toLocaleLowerCase() === 'v' && this.clipboard) {
      event.preventDefault();
      const center = screenToWorld(
        { x: this.view.canvas.width / 2, y: this.view.canvas.height / 2 },
        this.viewport,
      );
      const pasted = pasteEventNodes(
        this.clipboard,
        this.document.nodes.map((node) => node.id),
        center.x,
        center.y,
      );
      const first = this.document.nodeCount;
      for (const node of pasted) this.document.insertNode(node);
      this.selectedNodes = new Set(pasted.map((_, offset) => first + offset));
      this.commitDocument();
    }
  };

  private ensureCanvasSize(): void {
    const width = Math.max(320, Math.round(this.view.canvas.clientWidth));
    const height = Math.max(320, Math.round(this.view.canvas.clientHeight));
    if (this.view.canvas.width !== width) this.view.canvas.width = width;
    if (this.view.canvas.height !== height) this.view.canvas.height = height;
  }

  private readonly draw = (): void => {
    if (this.destroyed) return;
    this.ensureCanvasSize();
    const context = this.view.canvas.getContext('2d');
    if (context) {
      this.renderer.draw(context, {
        nodes: this.document?.nodes ?? [],
        selectedIndexes: this.selectedNodes,
        viewport: this.viewport,
        selectionStart:
          this.drag?.kind === 'select' ? this.drag.world : undefined,
        selectionEnd: this.selectionEnd,
      });
    }
    this.animationFrame = requestAnimationFrame(this.draw);
  };

  private updateCanvasStatus(): void {
    this.view.canvasStatus.textContent = selectionStatus(
      this.selectedNodes.size,
      this.document?.nodeCount ?? 0,
    );
    this.view.deleteNodesButton.disabled = this.selectedNodes.size === 0;
  }

  private readRecent(): string[] {
    try {
      const value = JSON.parse(
        localStorage.getItem('ceditor2.recentEvents') ?? '[]',
      ) as unknown;
      return Array.isArray(value)
        ? value.filter((item): item is string => typeof item === 'string')
        : [];
    } catch {
      return [];
    }
  }

  private writeRecent(id: string): void {
    try {
      localStorage.setItem(
        'ceditor2.recentEvents',
        JSON.stringify(
          [id, ...this.readRecent().filter((item) => item !== id)].slice(0, 8),
        ),
      );
    } catch {
      // Recent selections are a convenience; editing must work without storage.
    }
  }
}
