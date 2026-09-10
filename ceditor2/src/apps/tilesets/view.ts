import type { JsonArray } from '../../core/database/index.js';
import {
  TERRAIN_BORDER_TAGS,
  TILE_STEP_SOUNDS,
  cloneTilesetRecord,
  createDefaultTile,
  createDefaultTileset,
  createUniqueTilesetName,
  parseTilesetCollection,
  reconcileTilesToDimensions,
  stepSoundNumber,
  type TerrainBorderTag,
  type TileMetadata,
  type TilesetRecord,
} from '../../core/domain/tilesets/index.js';
import { element } from '../../core/ui/dom.js';
import {
  createEntityPicturePreview,
  createMediaPickerField,
  type DatabasePageContext,
} from '../../core/ui/index.js';
import {
  findDeepLinkedTileset,
  matchesTilesetSearch,
  withTilesetSelection,
} from './editorModel.js';

function field(
  labelText: string,
  id: string,
  control: HTMLInputElement | HTMLSelectElement | HTMLTextAreaElement,
  help?: string,
): HTMLDivElement {
  const wrapper = element('div', { className: 'field' });
  control.id = id;
  wrapper.append(
    element('label', { text: labelText, attributes: { for: id } }),
    control,
  );
  if (help) {
    const helpId = `${id}-help`;
    control.setAttribute('aria-describedby', helpId);
    wrapper.append(
      element('span', {
        className: 'field__help',
        text: help,
        attributes: { id: helpId },
      }),
    );
  }
  return wrapper;
}

function textInput(value: string): HTMLInputElement {
  const input = element('input');
  input.type = 'text';
  input.value = value;
  input.autocomplete = 'off';
  return input;
}

function numberInput(value: number): HTMLInputElement {
  const input = element('input');
  input.type = 'number';
  input.step = '1';
  input.value = String(value);
  return input;
}

function button(
  text: string,
  onClick: () => void,
  variant = '',
): HTMLButtonElement {
  const result = element('button', {
    className: `button${variant ? ` ${variant}` : ''}`,
    text,
    attributes: { type: 'button' },
  });
  result.addEventListener('click', onClick);
  return result;
}

function section(title: string, description?: string): HTMLFieldSetElement {
  const result = element('fieldset', { className: 'form-section' });
  result.append(element('legend', { text: title }));
  if (description) {
    result.append(
      element('p', {
        className: 'form-section__description',
        text: description,
      }),
    );
  }
  return result;
}

function checkbox(
  id: string,
  labelText: string,
  checked: boolean,
  onChange: (value: boolean) => void,
): HTMLDivElement {
  const wrapper = element('div', { className: 'check-field' });
  const input = element('input');
  input.type = 'checkbox';
  input.id = id;
  input.checked = checked;
  input.addEventListener('change', () => onChange(input.checked));
  wrapper.append(
    input,
    element('label', { text: labelText, attributes: { for: id } }),
  );
  return wrapper;
}

function terrainSelect(value: string): HTMLSelectElement {
  const select = element('select');
  const values = TERRAIN_BORDER_TAGS.includes(value as never)
    ? TERRAIN_BORDER_TAGS
    : ([value, ...TERRAIN_BORDER_TAGS] as readonly string[]);
  for (const terrain of values) {
    const option = element('option', { text: terrain });
    option.value = terrain;
    select.append(option);
  }
  select.value = value;
  return select;
}

export class TilesetsEditor {
  private records: TilesetRecord[];
  private selectedIndex: number;
  private selectedTileIndex = 0;
  private searchTerm = '';
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly resultCount = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseTilesetCollection(
      context.session.collection('tilesets'),
    );
    this.selectedIndex = findDeepLinkedTileset(this.records, initialUrl);
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', {
      className: 'editor-sidebar',
      attributes: { 'aria-label': 'Tilesets' },
    });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Name or sprite base';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      field('Search tilesets', 'tileset-search', search),
      button('+ New Tileset', () => this.createRecord(), 'button--primary'),
      this.resultCount,
    );
    sidebar.append(controls, this.list);
    layout.append(sidebar, this.main);
    this.root.replaceChildren(layout);
    this.renderList();
    this.renderForm();
  }

  destroy(): void {
    this.root.replaceChildren();
  }

  private selected(): TilesetRecord | undefined {
    return this.records[this.selectedIndex];
  }

  private commit(next?: TilesetRecord): void {
    if (next && this.selectedIndex >= 0)
      this.records[this.selectedIndex] = next;
    this.context.session.replaceCollection(
      'tilesets',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
  }

  private setSelection(index: number): void {
    this.selectedIndex = index;
    this.selectedTileIndex = 0;
    window.history.replaceState(
      null,
      '',
      withTilesetSelection(
        new URL(window.location.href),
        this.selected()?.name,
      ),
    );
    this.renderList();
    this.renderForm();
  }

  private createRecord(): void {
    const names = this.records.map(({ name }) => name);
    const name = names.includes('NEW_TILESET')
      ? createUniqueTilesetName('NEW_TILESET', names)
      : 'NEW_TILESET';
    this.records.push(createDefaultTileset(name));
    this.searchTerm = '';
    const search = this.root.querySelector<HTMLInputElement>('#tileset-search');
    if (search) search.value = '';
    this.commit();
    this.setSelection(this.records.length - 1);
  }

  private cloneSelected(): void {
    const current = this.selected();
    if (!current) return;
    const clone = cloneTilesetRecord(
      current,
      this.records.map(({ name }) => name),
    );
    const index = this.selectedIndex + 1;
    this.records.splice(index, 0, clone);
    this.commit();
    this.setSelection(index);
  }

  private deleteSelected(): void {
    const current = this.selected();
    if (
      !current ||
      !window.confirm(
        `Delete tileset "${current.name}"? This is saved when you use Save All.`,
      )
    ) {
      return;
    }
    this.records.splice(this.selectedIndex, 1);
    this.selectedIndex = -1;
    this.commit();
    this.setSelection(-1);
  }

  private updateRecord(
    update: (record: TilesetRecord) => TilesetRecord,
    renderForm = false,
  ): void {
    const current = this.selected();
    if (!current) return;
    const next = update(current);
    this.commit(next);
    window.history.replaceState(
      null,
      '',
      withTilesetSelection(new URL(window.location.href), next.name),
    );
    this.renderList();
    if (renderForm) this.renderForm();
  }

  private updateTile(
    update: (tile: TileMetadata) => TileMetadata,
    renderForm = false,
  ): void {
    this.updateRecord((record) => {
      const tiles = [...(record.tiles ?? [])];
      const current = tiles[this.selectedTileIndex];
      if (!current) return record;
      tiles[this.selectedTileIndex] = update(current);
      return { ...record, tiles };
    }, renderForm);
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matching = this.records
      .map((record, index) => ({ record, index }))
      .filter(({ record }) => matchesTilesetSearch(record, this.searchTerm));
    this.resultCount.textContent = `${matching.length} of ${this.records.length}`;
    if (!matching.length) {
      this.list.append(
        element('li', { className: 'empty-state', text: 'No tilesets found.' }),
      );
      return;
    }
    for (const { record, index } of matching) {
      const item = element('li', { className: 'entity-list__item' });
      const select = element('button', {
        className: 'entity-card entity-card--media',
        attributes: {
          type: 'button',
          'aria-current': String(index === this.selectedIndex),
        },
      });
      const body = element('span', { className: 'entity-card__body' });
      body.append(
        element('span', {
          className: 'entity-card__title',
          text: record.name || '(unnamed tileset)',
        }),
        element('span', {
          className: 'entity-card__subtitle',
          text: `${record.spriteBase || 'No sprite'} · ${record.tiles?.length ?? 0} tiles`,
        }),
      );
      select.append(createEntityPicturePreview(record.spriteBase ?? ''), body);
      select.addEventListener('click', () => this.setSelection(index));
      item.append(select);
      this.list.append(item);
    }
  }

  private renderForm(): void {
    this.main.replaceChildren();
    const current = this.selected();
    if (!current) {
      this.main.append(
        element('div', {
          className: 'empty-state',
          text: 'Select a tileset to edit, or create a new one.',
        }),
      );
      return;
    }
    const form = element('form', {
      className: 'editor-form',
      attributes: { novalidate: '' },
    });
    form.addEventListener('submit', (event) => event.preventDefault());
    form.append(this.renderIdentity(current), this.renderTiles(current));
    const actions = element('div', { className: 'form-actions' });
    actions.append(
      element('span', {
        className: 'dirty-indicator',
        text: 'Changes remain local until Save All.',
      }),
      button('Clone', () => this.cloneSelected()),
      button('Delete', () => this.deleteSelected(), 'button--danger'),
    );
    form.append(actions);
    this.main.append(form);
  }

  private renderIdentity(current: TilesetRecord): HTMLFieldSetElement {
    const result = section(
      'Tileset',
      'Image dimensions describe the sheet; tile dimensions describe each sprite cell.',
    );
    const grid = element('div', { className: 'form-grid' });
    const name = textInput(current.name);
    name.required = true;
    name.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, name: name.value })),
    );
    grid.append(
      field('Name (ID)', 'tileset-name', name),
      createMediaPickerField({
        id: 'tileset-sprite-base',
        label: 'Sprite base',
        kind: 'picture',
        value: current.spriteBase ?? '',
        help: 'Choose from the game picture catalog or enter a legacy alias.',
        onChange: (value) =>
          this.updateRecord((record) => ({
            ...record,
            spriteBase: value,
          })),
      }),
    );
    for (const [key, label] of [
      ['imageWidth', 'Image width'],
      ['imageHeight', 'Image height'],
      ['tileWidth', 'Tile width'],
      ['tileHeight', 'Tile height'],
    ] as const) {
      const input = numberInput(current[key] ?? 0);
      input.min = key.startsWith('tile') ? '1' : '0';
      input.addEventListener('input', () => {
        if (input.value === '' || !Number.isInteger(input.valueAsNumber))
          return;
        this.updateRecord((record) => ({
          ...record,
          [key]: input.valueAsNumber,
        }));
      });
      grid.append(field(label, `tileset-${key}`, input));
    }
    const tileWidth = current.tileWidth ?? 0;
    const tileHeight = current.tileHeight ?? 0;
    const expected =
      tileWidth > 0 && tileHeight > 0
        ? Math.floor((current.imageWidth ?? 0) / tileWidth) *
          Math.floor((current.imageHeight ?? 0) / tileHeight)
        : 0;
    result.append(
      grid,
      element('p', {
        className: 'tileset-inline-status',
        text: `Dimensions describe ${expected} cells; metadata currently has ${current.tiles?.length ?? 0} entries.`,
      }),
      button('Fit metadata to dimensions', () => {
        const currentCount = this.selected()?.tiles?.length ?? 0;
        if (
          expected < currentCount &&
          !window.confirm(
            `Remove ${currentCount - expected} trailing tile records?`,
          )
        ) {
          return;
        }
        this.selectedTileIndex = Math.max(
          0,
          Math.min(this.selectedTileIndex, expected - 1),
        );
        this.updateRecord(reconcileTilesToDimensions, true);
      }),
    );
    return result;
  }

  private renderTiles(current: TilesetRecord): HTMLFieldSetElement {
    const result = section(
      'Tile metadata',
      'Tile IDs and properties are stored independently from the sprite sheet.',
    );
    const controls = element('div', { className: 'button-row' });
    controls.append(
      button('+ Add tile', () => {
        const ids = new Set((current.tiles ?? []).map(({ id }) => id));
        let id = 0;
        while (ids.has(id)) id += 1;
        const nextIndex = current.tiles?.length ?? 0;
        this.selectedTileIndex = nextIndex;
        this.updateRecord(
          (record) => ({
            ...record,
            tiles: [...(record.tiles ?? []), createDefaultTile(id)],
          }),
          true,
        );
      }),
      button(
        'Remove selected tile',
        () => this.removeSelectedTile(),
        'button--danger',
      ),
    );
    const browser = element('div', { className: 'tileset-tile-browser' });
    const list = element('ol', { className: 'tileset-tile-list' });
    const tiles = current.tiles ?? [];
    if (this.selectedTileIndex >= tiles.length) {
      this.selectedTileIndex = Math.max(0, tiles.length - 1);
    }
    for (const [index, tile] of tiles.entries()) {
      const item = element('li');
      const select = element('button', {
        attributes: {
          type: 'button',
          'aria-current': String(index === this.selectedTileIndex),
        },
      });
      select.append(
        element('span', { className: 'tileset-tile-id', text: `#${tile.id}` }),
        element('span', {
          className: 'tileset-tile-description',
          text: tile.description || 'No description',
        }),
      );
      select.addEventListener('click', () => {
        this.selectedTileIndex = index;
        this.renderForm();
      });
      item.append(select);
      list.append(item);
    }
    const editor = element('div', { className: 'tileset-tile-editor' });
    const tile = tiles[this.selectedTileIndex];
    if (tile) {
      editor.append(this.renderTileEditor(tile));
    } else {
      editor.append(
        element('p', {
          className: 'empty-state',
          text: 'Add a tile to edit metadata.',
        }),
      );
    }
    browser.append(list, editor);
    result.append(controls, browser);
    return result;
  }

  private renderTileEditor(tile: TileMetadata): DocumentFragment {
    const fragment = document.createDocumentFragment();
    const grid = element('div', { className: 'form-grid' });
    const id = numberInput(tile.id);
    id.addEventListener('input', () => {
      if (id.value !== '' && Number.isInteger(id.valueAsNumber)) {
        this.updateTile((record) => ({ ...record, id: id.valueAsNumber }));
      }
    });
    const description = textInput(tile.description ?? '');
    description.addEventListener('input', () =>
      this.updateTile((record) => ({
        ...record,
        description: description.value,
      })),
    );
    const stepSound = element('select');
    for (const sound of TILE_STEP_SOUNDS) {
      const option = element('option', { text: sound.label });
      option.value = String(sound.value);
      stepSound.append(option);
    }
    stepSound.value = String(stepSoundNumber(tile.stepSound));
    stepSound.addEventListener('change', () =>
      this.updateTile((record) => ({
        ...record,
        stepSound: Number(stepSound.value),
      })),
    );
    grid.append(
      field('Tile ID', 'tileset-tile-id', id),
      field('Description', 'tileset-tile-description', description),
      field('Step sound', 'tileset-tile-step-sound', stepSound),
    );
    const flags = element('div', { className: 'form-grid' });
    for (const [key, label, fallback] of [
      ['isWalkable', 'Walkable', true],
      ['isSeeThrough', 'See through', true],
      ['isDoor', 'Door', false],
      ['isContainer', 'Container', false],
    ] as const) {
      flags.append(
        checkbox(
          `tileset-tile-${key}`,
          label,
          tile[key] ?? fallback,
          (checked) =>
            this.updateTile((record) => ({ ...record, [key]: checked })),
        ),
      );
    }
    fragment.append(grid, flags, this.renderTerrainCorners(tile));
    return fragment;
  }

  private renderTerrainCorners(tile: TileMetadata): HTMLDivElement {
    const wrapper = element('div', {
      className: 'tileset-corners form-stack',
    });
    wrapper.append(
      checkbox(
        'tileset-tile-terrain-enabled',
        'Store terrain corner metadata',
        tile.tileTerrainBorderMeta !== undefined,
        (enabled) =>
          this.updateTile((record) => {
            if (enabled) {
              return {
                ...record,
                tileTerrainBorderMeta: {
                  nw: 'NONE',
                  ne: 'NONE',
                  sw: 'NONE',
                  se: 'NONE',
                },
              };
            }
            const next = structuredClone(record);
            delete next.tileTerrainBorderMeta;
            return next;
          }, true),
      ),
    );
    if (!tile.tileTerrainBorderMeta) return wrapper;
    const grid = element('div', { className: 'form-grid' });
    for (const corner of ['nw', 'ne', 'sw', 'se'] as const) {
      const select = terrainSelect(tile.tileTerrainBorderMeta[corner]);
      select.addEventListener('change', () =>
        this.updateTile((record) => ({
          ...record,
          tileTerrainBorderMeta: {
            ...record.tileTerrainBorderMeta!,
            [corner]: select.value as TerrainBorderTag,
          },
        })),
      );
      grid.append(
        field(corner.toUpperCase(), `tileset-tile-terrain-${corner}`, select),
      );
    }
    wrapper.append(grid);
    return wrapper;
  }

  private removeSelectedTile(): void {
    const current = this.selected();
    const tile = current?.tiles?.[this.selectedTileIndex];
    if (!current || !tile || !window.confirm(`Remove tile #${tile.id}?`))
      return;
    const tiles = [...(current.tiles ?? [])];
    tiles.splice(this.selectedTileIndex, 1);
    this.selectedTileIndex = Math.max(
      0,
      Math.min(this.selectedTileIndex, tiles.length - 1),
    );
    this.updateRecord((record) => ({ ...record, tiles }), true);
  }
}
