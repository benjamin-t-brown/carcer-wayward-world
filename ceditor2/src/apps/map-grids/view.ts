import type { JsonArray } from '../../core/database/index.js';
import {
  createBlankMapGrid,
  parseMapGridCollection,
  type MapGridRecord,
} from '../../core/domain/mapGrids/index.js';
import type { MapRecord } from '../../core/domain/maps/index.js';
import { element } from '../../core/ui/dom.js';
import type { DatabasePageContext } from '../../core/ui/index.js';
import {
  createUniqueGridName,
  estimateMapGridCost,
  matchesMapGridSearch,
} from './editorModel.js';

export class MapGridsEditor {
  private grids: MapGridRecord[];
  private maps: MapRecord[];
  private selectedIndex = -1;
  private searchTerm = '';
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly count = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
  ) {
    this.grids = parseMapGridCollection(context.session.collection('mapGrids'));
    this.maps = context.session.collection('maps') as MapRecord[];
    this.selectedIndex = this.grids.length ? 0 : -1;
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', { className: 'editor-sidebar' });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = input('search', 'Search grids');
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      labeled('Search map grids', 'map-grid-search', search),
      actionButton('+ New Grid', () => this.showCreateForm(), true),
      this.count,
    );
    sidebar.append(controls, this.list);
    layout.append(sidebar, this.main);
    this.root.replaceChildren(layout);
    this.renderList();
    this.renderSelected();
  }

  destroy(): void {
    this.root.replaceChildren();
  }

  private selected(): MapGridRecord | undefined {
    return this.grids[this.selectedIndex];
  }

  private commitGrids(): void {
    this.context.session.replaceCollection('mapGrids', this.grids as JsonArray);
    this.context.notifyChanged();
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matches = this.grids
      .map((grid, index) => ({ grid, index }))
      .filter(({ grid }) => matchesMapGridSearch(grid, this.searchTerm));
    this.count.textContent = `${matches.length} of ${this.grids.length}`;
    for (const { grid, index } of matches) {
      const button = element('button', {
        className: 'entity-card',
        attributes: {
          type: 'button',
          'aria-current': String(index === this.selectedIndex),
        },
      });
      const body = element('span', { className: 'entity-card__body' });
      body.append(
        element('span', { className: 'entity-card__title', text: grid.name }),
        element('span', {
          className: 'entity-card__subtitle',
          text: `${grid.gridWidth}×${grid.gridHeight} partitions`,
        }),
      );
      button.append(body);
      button.addEventListener('click', () => {
        this.selectedIndex = index;
        this.renderList();
        this.renderSelected();
      });
      const item = element('li', { className: 'entity-list__item' });
      item.append(button);
      this.list.append(item);
    }
  }

  private renderSelected(): void {
    this.main.replaceChildren();
    const grid = this.selected();
    if (!grid) {
      this.main.append(
        element('div', {
          className: 'empty-state',
          text: 'Select a grid, or create one with its blank map partitions.',
        }),
      );
      return;
    }
    const form = element('form', { className: 'editor-form' });
    form.addEventListener('submit', (event) => event.preventDefault());
    const identity = section('Grid properties');
    const name = input('text');
    name.value = grid.name;
    const label = input('text');
    label.value = grid.label ?? '';
    name.addEventListener('change', () => {
      const next = name.value.trim();
      if (
        !next ||
        this.grids.some(
          (candidate, index) =>
            index !== this.selectedIndex && candidate.name === next,
        )
      ) {
        name.setCustomValidity('Grid API names must be non-empty and unique.');
        name.reportValidity();
        return;
      }
      name.setCustomValidity('');
      grid.name = next;
      this.commitGrids();
      this.renderList();
    });
    label.addEventListener('input', () => {
      grid.label = label.value;
      this.commitGrids();
      this.renderList();
    });
    const dimensions = element('div', { className: 'form-grid' });
    dimensions.append(
      readonlyValue('Grid width', grid.gridWidth),
      readonlyValue('Grid height', grid.gridHeight),
      readonlyValue('Partition width', grid.mapWidth),
      readonlyValue('Partition height', grid.mapHeight),
    );
    identity.append(
      gridFields(
        labeled('API name', 'map-grid-name', name),
        labeled('Label', 'map-grid-label', label),
      ),
      dimensions,
      element('p', {
        className: 'field__help',
        text: 'Dimensions are fixed after creation so existing map tile arrays are never silently resized. Deleting a grid leaves its maps available as standalone maps.',
      }),
    );

    const partitions = this.renderPartitionInspector(grid);
    const actions = element('div', { className: 'form-actions' });
    actions.append(
      actionButton('Delete Grid', () => this.deleteSelected(), false, true),
    );
    form.append(identity, partitions, actions);
    this.main.append(form);
  }

  private renderPartitionInspector(grid: MapGridRecord): HTMLElement {
    const result = section('Backing partitions');
    result.append(
      element('p', {
        className: 'form-section__description',
        text: 'The canvas treats these maps as one continuous workspace. Choose a coordinate only when you need its direct map metadata.',
      }),
    );
    const x = input('number');
    x.min = '0';
    x.max = String(grid.gridWidth - 1);
    x.value = '0';
    const y = input('number');
    y.min = '0';
    y.max = String(grid.gridHeight - 1);
    y.value = '0';
    const output = element('div', { className: 'map-grid-partition' });
    const update = () => {
      const cellX = clampCoordinate(x.valueAsNumber, grid.gridWidth);
      const cellY = clampCoordinate(y.valueAsNumber, grid.gridHeight);
      x.value = String(cellX);
      y.value = String(cellY);
      const mapName = grid.cells[cellY]?.[cellX] ?? '';
      output.replaceChildren(
        element('code', { text: mapName || '(unassigned)' }),
      );
      if (mapName) {
        const link = element('a', { text: 'Open in map workspace' });
        link.href = `/pages/maps/?map=${encodeURIComponent(mapName)}`;
        output.append(link);
      }
    };
    x.addEventListener('input', update);
    y.addEventListener('input', update);
    result.append(
      gridFields(
        labeled('Column (x)', 'map-grid-cell-x', x),
        labeled('Row (y)', 'map-grid-cell-y', y),
      ),
      output,
    );
    update();
    return result;
  }

  private showCreateForm(): void {
    this.main.replaceChildren();
    const form = element('form', { className: 'editor-form' });
    const create = section('Create continuous map grid');
    const name = input('text');
    name.required = true;
    name.value = createUniqueGridName(
      'NEW_MAP_GRID',
      this.grids.map((grid) => grid.name),
    );
    const label = input('text');
    label.value = 'New Map Grid';
    const gridWidth = positiveNumber(3);
    const gridHeight = positiveNumber(3);
    const mapWidth = positiveNumber(30);
    const mapHeight = positiveNumber(30);
    const estimate = element('p', { className: 'status status--info' });
    const updateEstimate = () => {
      const cost = estimateMapGridCost(
        gridWidth.valueAsNumber,
        gridHeight.valueAsNumber,
        mapWidth.valueAsNumber,
        mapHeight.valueAsNumber,
      );
      estimate.textContent = `${cost.partitions.toLocaleString()} maps · ${cost.tiles.toLocaleString()} tiles · ${cost.denseValues.toLocaleString()} dense values`;
    };
    for (const control of [gridWidth, gridHeight, mapWidth, mapHeight]) {
      control.addEventListener('input', updateEstimate);
    }
    create.append(
      gridFields(
        labeled('Grid API name', 'new-map-grid-name', name),
        labeled('Label', 'new-map-grid-label', label),
        labeled('Partitions wide', 'new-map-grid-width', gridWidth),
        labeled('Partitions high', 'new-map-grid-height', gridHeight),
        labeled('Map tiles wide', 'new-map-width', mapWidth),
        labeled('Map tiles high', 'new-map-height', mapHeight),
      ),
      estimate,
      element('p', {
        className: 'field__help',
        text: 'Every cell receives a blank map with a random stable API name. Maps and grid are staged together for Save All.',
      }),
    );
    const actions = element('div', { className: 'form-actions' });
    actions.append(
      actionButton('Cancel', () => this.renderSelected()),
      actionButton('Create Grid and Maps', () => undefined, true),
    );
    form.append(create, actions);
    form.addEventListener('submit', (event) => {
      event.preventDefault();
      const requestedName = name.value.trim();
      if (this.grids.some((grid) => grid.name === requestedName)) {
        name.setCustomValidity('This grid API name already exists.');
        name.reportValidity();
        return;
      }
      name.setCustomValidity('');
      const cost = estimateMapGridCost(
        gridWidth.valueAsNumber,
        gridHeight.valueAsNumber,
        mapWidth.valueAsNumber,
        mapHeight.valueAsNumber,
      );
      if (
        cost.partitions > 100 &&
        !window.confirm(
          `Create ${cost.partitions.toLocaleString()} backing maps now?`,
        )
      )
        return;
      const created = createBlankMapGrid({
        name: requestedName,
        label: label.value,
        gridWidth: gridWidth.valueAsNumber,
        gridHeight: gridHeight.valueAsNumber,
        mapWidth: mapWidth.valueAsNumber,
        mapHeight: mapHeight.valueAsNumber,
        existingMapNames: this.maps.map((map) => map.name),
      });
      this.maps.push(...created.maps);
      this.grids.push(created.grid);
      this.context.session.replaceCollection('maps', this.maps as JsonArray);
      this.context.session.replaceCollection(
        'mapGrids',
        this.grids as JsonArray,
      );
      this.context.notifyChanged();
      this.selectedIndex = this.grids.length - 1;
      this.searchTerm = '';
      this.renderList();
      this.renderSelected();
    });
    this.main.append(form);
    updateEstimate();
  }

  private deleteSelected(): void {
    const grid = this.selected();
    if (
      !grid ||
      !window.confirm(
        `Delete grid "${grid.name}"? Its backing maps will remain as standalone maps.`,
      )
    )
      return;
    this.grids.splice(this.selectedIndex, 1);
    this.selectedIndex = Math.min(this.selectedIndex, this.grids.length - 1);
    this.commitGrids();
    this.renderList();
    this.renderSelected();
  }
}

function input(type: string, placeholder = ''): HTMLInputElement {
  const control = element('input');
  control.type = type;
  control.placeholder = placeholder;
  return control;
}

function positiveNumber(value: number): HTMLInputElement {
  const control = input('number');
  control.required = true;
  control.min = '1';
  control.step = '1';
  control.value = String(value);
  return control;
}

function labeled(
  label: string,
  id: string,
  control: HTMLInputElement,
): HTMLElement {
  control.id = id;
  const wrapper = element('div', { className: 'field' });
  wrapper.append(
    element('label', { text: label, attributes: { for: id } }),
    control,
  );
  return wrapper;
}

function readonlyValue(label: string, value: string | number): HTMLElement {
  const control = input('text');
  control.value = String(value);
  control.readOnly = true;
  return labeled(label, `grid-readonly-${label.replaceAll(' ', '-')}`, control);
}

function gridFields(...children: HTMLElement[]): HTMLElement {
  const grid = element('div', { className: 'form-grid' });
  grid.append(...children);
  return grid;
}

function section(title: string): HTMLFieldSetElement {
  const fieldset = element('fieldset', { className: 'form-section' });
  fieldset.append(element('legend', { text: title }));
  return fieldset;
}

function actionButton(
  text: string,
  action: () => void,
  primary = false,
  danger = false,
): HTMLButtonElement {
  const button = element('button', {
    className: `button${primary ? ' button--primary' : ''}${danger ? ' button--danger' : ''}`,
    text,
    attributes: { type: primary ? 'submit' : 'button' },
  });
  button.addEventListener('click', action);
  return button;
}

function clampCoordinate(value: number, size: number): number {
  return Math.max(0, Math.min(size - 1, Math.floor(value) || 0));
}
