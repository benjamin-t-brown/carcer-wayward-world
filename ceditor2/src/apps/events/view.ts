import { element } from '../../core/ui/dom.js';

export interface EventEditorElements {
  readonly layout: HTMLElement;
  readonly search: HTMLInputElement;
  readonly list: HTMLUListElement;
  readonly resultCount: HTMLElement;
  readonly newButton: HTMLButtonElement;
  readonly canvas: HTMLCanvasElement;
  readonly canvasStatus: HTMLElement;
  readonly fitButton: HTMLButtonElement;
  readonly findNodeInput: HTMLInputElement;
  readonly findNodeButton: HTMLButtonElement;
  readonly addButtons: ReadonlyMap<string, HTMLButtonElement>;
  readonly deleteNodesButton: HTMLButtonElement;
  readonly inspector: HTMLElement;
}

function button(text: string, className = 'button'): HTMLButtonElement {
  return element('button', {
    className,
    text,
    attributes: { type: 'button' },
  });
}

export function createEventEditorView(): EventEditorElements {
  const layout = element('div', { className: 'event-editor' });
  const sidebar = element('aside', {
    className: 'event-editor__sidebar',
    attributes: { 'aria-label': 'Special events' },
  });
  const sidebarControls = element('div', {
    className: 'event-editor__sidebar-controls',
  });
  const search = element('input');
  search.id = 'event-search';
  search.type = 'search';
  search.placeholder = 'ID, title, or type';
  const searchLabel = element('label', {
    text: 'Search events',
    attributes: { for: search.id },
  });
  const newButton = button('+ New Event', 'button button--primary');
  const resultCount = element('p', { className: 'muted' });
  sidebarControls.append(searchLabel, search, newButton, resultCount);
  const list = element('ul', { className: 'event-editor__list' });
  sidebar.append(sidebarControls, list);

  const workspace = element('section', { className: 'event-workspace' });
  const toolbar = element('div', { className: 'event-toolbar' });
  const fitButton = button('Fit graph');
  const findNodeInput = element('input', {
    className: 'event-toolbar__find',
    attributes: { type: 'search', placeholder: 'Exact node ID' },
  });
  const findNodeButton = button('Find node', 'button button--small');
  toolbar.append(fitButton, findNodeInput, findNodeButton);
  const addButtons = new Map<string, HTMLButtonElement>();
  for (const type of [
    'EXEC',
    'CHOICE',
    'SWITCH',
    'END',
    'COMMENT',
    'KEYWORD',
  ]) {
    const addButton = button(`+ ${type}`, 'button button--small');
    addButtons.set(type, addButton);
    toolbar.append(addButton);
  }
  const deleteNodesButton = button(
    'Delete selected',
    'button button--small button--danger',
  );
  toolbar.append(deleteNodesButton);
  const canvasWrap = element('div', { className: 'event-canvas-wrap' });
  const canvas = element('canvas', {
    className: 'event-canvas',
    attributes: {
      tabindex: '0',
      'aria-label':
        'Event graph. Drag nodes to move. Drag empty space to select. Middle-drag or Alt-drag to pan. Wheel to zoom.',
    },
  });
  const canvasStatus = element('p', {
    className: 'event-canvas-status',
    text: 'Select an event to edit its graph.',
  });
  canvasWrap.append(canvas, canvasStatus);
  workspace.append(toolbar, canvasWrap);

  const inspector = element('aside', {
    className: 'event-inspector',
    attributes: { 'aria-label': 'Event and node inspector' },
  });
  layout.append(sidebar, workspace, inspector);
  return {
    layout,
    search,
    list,
    resultCount,
    newButton,
    canvas,
    canvasStatus,
    fitButton,
    findNodeInput,
    findNodeButton,
    addButtons,
    deleteNodesButton,
    inspector,
  };
}
