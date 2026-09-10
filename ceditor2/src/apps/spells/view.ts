import type { JsonArray } from '../../core/database/index.js';
import {
  RUNE_TYPES,
  cloneSpellRecord,
  createSpellRecord,
  createUniqueSpellName,
  parseSpellCollection,
  type RuneType,
  type SpellRecord,
  type SpellRuneRequirement,
} from '../../core/domain/spells/index.js';
import { element } from '../../core/ui/dom.js';
import {
  createEntitySpritePreview,
  createMediaPickerField,
  type DatabasePageContext,
} from '../../core/ui/index.js';
import {
  findDeepLinkedSpell,
  matchesSpellSearch,
  withSpellSelection,
} from './editorModel.js';

type AbilityOption = { label: string; name: string };

function abilityOption(value: unknown): AbilityOption | null {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return null;
  }
  const record = value as Record<string, unknown>;
  if (typeof record.name !== 'string' || !record.name.trim()) return null;
  return {
    name: record.name,
    label:
      typeof record.label === 'string' && record.label.trim()
        ? record.label
        : record.name,
  };
}

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

export class SpellsEditor {
  private records: SpellRecord[];
  private selectedIndex: number;
  private searchTerm = '';
  private readonly abilities: AbilityOption[];
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly resultCount = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseSpellCollection(context.session.collection('spells'));
    this.selectedIndex = findDeepLinkedSpell(this.records, initialUrl);
    this.abilities = context.session
      .collection('abilities')
      .map(abilityOption)
      .filter((value): value is AbilityOption => value !== null)
      .sort((left, right) => left.name.localeCompare(right.name));
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', {
      className: 'editor-sidebar',
      attributes: { 'aria-label': 'Spells' },
    });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Name, label, description, or ability';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      field('Search spells', 'spell-search', search),
      button('+ New Spell', () => this.createRecord(), 'button--primary'),
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

  private selected(): SpellRecord | undefined {
    return this.records[this.selectedIndex];
  }

  private commit(nextRecord?: SpellRecord): void {
    if (nextRecord && this.selectedIndex >= 0) {
      this.records[this.selectedIndex] = nextRecord;
    }
    this.context.session.replaceCollection('spells', this.records as JsonArray);
    this.context.notifyChanged();
  }

  private setSelection(index: number): void {
    this.selectedIndex = index;
    const nextUrl = withSpellSelection(
      new URL(window.location.href),
      this.selected()?.name,
    );
    window.history.replaceState(null, '', nextUrl);
    this.renderList();
    this.renderForm();
  }

  private createRecord(): void {
    const names = this.records.map((record) => record.name);
    const name = names.includes('NEW_SPELL')
      ? createUniqueSpellName('NEW_SPELL', names)
      : 'NEW_SPELL';
    this.records.push(createSpellRecord(name));
    this.clearSearch();
    this.commit();
    this.setSelection(this.records.length - 1);
  }

  private cloneSelected(): void {
    const current = this.selected();
    if (!current) return;
    const clone = cloneSpellRecord(
      current,
      this.records.map((record) => record.name),
    );
    const insertAt = this.selectedIndex + 1;
    this.records.splice(insertAt, 0, clone);
    this.clearSearch();
    this.commit();
    this.setSelection(insertAt);
  }

  private clearSearch(): void {
    this.searchTerm = '';
    const search = this.root.querySelector<HTMLInputElement>('#spell-search');
    if (search) search.value = '';
  }

  private deleteSelected(): void {
    const current = this.selected();
    if (
      !current ||
      !window.confirm(
        `Delete spell "${current.name}"? This is saved when you use Save All.`,
      )
    ) {
      return;
    }
    this.records.splice(this.selectedIndex, 1);
    this.commit();
    this.setSelection(-1);
  }

  private updateRecord(
    update: (current: SpellRecord) => SpellRecord,
    renderForm = false,
  ): void {
    const current = this.selected();
    if (!current) return;
    const updated = update(current);
    this.commit(updated);
    window.history.replaceState(
      null,
      '',
      withSpellSelection(new URL(window.location.href), updated.name),
    );
    this.renderList();
    if (renderForm) this.renderForm();
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matching = this.records
      .map((record, index) => ({ index, record }))
      .filter(({ record }) => matchesSpellSearch(record, this.searchTerm));
    this.resultCount.textContent = `${matching.length} of ${this.records.length}`;
    if (matching.length === 0) {
      this.list.append(
        element('li', { className: 'empty-state', text: 'No spells found.' }),
      );
      return;
    }
    for (const { index, record } of matching) {
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
          text: record.label || record.name || '(unnamed spell)',
        }),
        element('span', {
          className: 'entity-card__subtitle',
          text: `(${record.name || 'No ID'})`,
        }),
      );
      select.append(createEntitySpritePreview(record.icon), body);
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
          text: 'Select a spell to edit, or create a new one.',
        }),
      );
      return;
    }
    const form = element('form', {
      className: 'editor-form',
      attributes: { novalidate: '' },
    });
    form.addEventListener('submit', (event) => event.preventDefault());
    form.append(this.renderIdentity(current), this.renderRunes(current));
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

  private renderIdentity(current: SpellRecord): HTMLFieldSetElement {
    const result = section('Spell');
    const grid = element('div', { className: 'form-grid' });
    const name = element('input');
    name.type = 'text';
    name.required = true;
    name.autocomplete = 'off';
    name.value = current.name;
    name.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, name: name.value })),
    );
    const label = element('input');
    label.type = 'text';
    label.required = true;
    label.value = current.label;
    label.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, label: label.value })),
    );
    grid.append(
      field('Name (ID)', 'spell-name', name),
      field('Display label', 'spell-label', label),
      createMediaPickerField({
        id: 'spell-icon',
        label: 'Icon sprite',
        kind: 'sprite',
        value: current.icon,
        required: true,
        onChange: (value) =>
          this.updateRecord((record) => ({ ...record, icon: value })),
      }),
    );

    const description = element('textarea');
    description.required = true;
    description.rows = 3;
    description.value = current.description;
    description.addEventListener('input', () =>
      this.updateRecord((record) => ({
        ...record,
        description: description.value,
      })),
    );

    const ability = element('input');
    ability.type = 'text';
    ability.required = true;
    ability.value = current.abilityName;
    ability.setAttribute('list', 'spell-ability-options');
    ability.autocomplete = 'off';
    ability.addEventListener('input', () =>
      this.updateRecord((record) => ({
        ...record,
        abilityName: ability.value,
      })),
    );
    const datalist = element('datalist', {
      attributes: { id: 'spell-ability-options' },
    });
    for (const option of this.abilities) {
      const item = element('option', { text: option.label });
      item.value = option.name;
      datalist.append(item);
    }
    const abilityWrapper = field(
      'Ability',
      'spell-ability',
      ability,
      'Searches the full abilities database. Save All reports missing references.',
    );
    abilityWrapper.append(datalist);
    if (current.abilityName) {
      abilityWrapper.append(
        element('a', {
          text: 'Edit linked ability',
          attributes: {
            href: `/pages/abilities/?ability=${encodeURIComponent(current.abilityName)}`,
          },
        }),
      );
    }
    result.append(
      grid,
      field('Description', 'spell-description', description),
      abilityWrapper,
    );
    return result;
  }

  private renderRunes(current: SpellRecord): HTMLFieldSetElement {
    const result = section(
      'Required runes',
      'A spell may use each rune type once. Counts must be positive integers.',
    );
    const list = element('div', { className: 'repeatable-list' });
    current.requiredRunes.forEach((requirement, index) =>
      list.append(this.renderRune(requirement, index, current.requiredRunes)),
    );
    if (current.requiredRunes.length === 0) {
      list.append(
        element('p', { className: 'muted', text: 'No runes required.' }),
      );
    }
    const unused = RUNE_TYPES.find(
      (type) => !current.requiredRunes.some((item) => item.type === type),
    );
    const add = button('+ Add Rune', () => {
      if (!unused) return;
      this.updateRecord(
        (record) => ({
          ...record,
          requiredRunes: [...record.requiredRunes, { type: unused, count: 1 }],
        }),
        true,
      );
    });
    add.disabled = unused === undefined;
    result.append(list, add);
    return result;
  }

  private renderRune(
    requirement: SpellRuneRequirement,
    index: number,
    all: readonly SpellRuneRequirement[],
  ): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    const header = element('div', { className: 'repeatable-item__header' });
    header.append(
      element('h3', {
        className: 'repeatable-item__title',
        text: `Rune ${index + 1}`,
      }),
      button(
        'Remove',
        () =>
          this.updateRecord(
            (record) => ({
              ...record,
              requiredRunes: record.requiredRunes.filter(
                (_, itemIndex) => itemIndex !== index,
              ),
            }),
            true,
          ),
        'button--danger button--small',
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const type = element('select');
    for (const runeType of RUNE_TYPES) {
      if (
        runeType !== requirement.type &&
        all.some((value) => value.type === runeType)
      ) {
        continue;
      }
      const option = element('option', { text: runeType });
      option.value = runeType;
      type.append(option);
    }
    type.value = requirement.type;
    type.addEventListener('change', () =>
      this.updateRune(index, (value) => ({
        ...value,
        type: type.value as RuneType,
      })),
    );
    const count = element('input');
    count.type = 'number';
    count.required = true;
    count.min = '1';
    count.step = '1';
    count.value = String(requirement.count);
    count.addEventListener('input', () => {
      if (
        count.value === '' ||
        !Number.isInteger(count.valueAsNumber) ||
        count.valueAsNumber <= 0
      ) {
        return;
      }
      this.updateRune(index, (value) => ({
        ...value,
        count: count.valueAsNumber,
      }));
    });
    grid.append(
      field('Rune type', `spell-rune-${index}-type`, type),
      field('Count', `spell-rune-${index}-count`, count),
    );
    item.append(header, grid);
    return item;
  }

  private updateRune(
    index: number,
    update: (value: SpellRuneRequirement) => SpellRuneRequirement,
  ): void {
    this.updateRecord((record) => ({
      ...record,
      requiredRunes: record.requiredRunes.map((value, itemIndex) =>
        itemIndex === index ? update(value) : value,
      ),
    }));
  }
}
