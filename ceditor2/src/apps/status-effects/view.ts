import type { JsonArray } from '../../core/database/index.js';
import {
  DAMAGE_TYPES,
  STAT_TYPES,
  STATUS_ACTION_TARGET_TYPES,
  STATUS_EFFECT_CONDITIONS,
  STATUS_EVENT_TYPES,
  cloneStatusEffectRecord,
  createDefaultDurationScale,
  createDefaultStatusAction,
  createStatusEffectRecord,
  createUniqueStatusEffectName,
  parseStatusEffectCollection,
  type StatusEffectCurrentStats,
  type StatusEffectResistance,
  type StatusEffectStats,
  type StatusEffectAction,
  type DamageType,
  type StatType,
  type StatusActionTargetType,
  type StatusEffectCondition,
  type StatusEffectEvent,
  type StatusEffectRecord,
  type StatusEventType,
} from '../../core/domain/statusEffects/index.js';
import { element } from '../../core/ui/dom.js';
import type { DatabasePageContext } from '../../core/ui/index.js';
import {
  findDeepLinkedStatusEffect,
  matchesStatusEffectSearch,
  withStatusEffectSelection,
} from './editorModel.js';

type SelectOption = { label?: string; value: string };

function recordName(value: unknown): string | null {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return null;
  }
  const name = (value as Record<string, unknown>).name;
  return typeof name === 'string' && name.trim() ? name : null;
}

function appendOptions(
  select: HTMLSelectElement,
  values: readonly (string | SelectOption)[],
  currentValue: string,
): void {
  const options = values.map((value) =>
    typeof value === 'string' ? { value } : value,
  );
  if (
    currentValue &&
    !options.some((option) => option.value === currentValue)
  ) {
    options.unshift({
      value: currentValue,
      label: `${currentValue} (unrecognized)`,
    });
  }

  for (const optionValue of options) {
    const option = element('option', {
      text: optionValue.label ?? optionValue.value,
    });
    option.value = optionValue.value;
    select.append(option);
  }
  select.value = currentValue;
}

function field(
  labelText: string,
  id: string,
  control: HTMLInputElement | HTMLSelectElement | HTMLTextAreaElement,
  help?: string,
): HTMLDivElement {
  const wrapper = element('div', { className: 'field' });
  const label = element('label', { text: labelText, attributes: { for: id } });
  control.id = id;
  wrapper.append(label, control);
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

function numberInput(value: number): HTMLInputElement {
  const input = element('input');
  input.type = 'number';
  input.value = String(value);
  return input;
}

function selectInput(
  values: readonly (string | SelectOption)[],
  value: string,
): HTMLSelectElement {
  const select = element('select');
  appendOptions(select, values, value);
  return select;
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
  onChange: (checked: boolean) => void,
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

export class StatusEffectsEditor {
  private records: StatusEffectRecord[];
  private selectedIndex: number;
  private searchTerm = '';
  private readonly abilityOptions: SelectOption[];
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly resultCount = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseStatusEffectCollection(
      context.session.collection('statusEffects'),
    );
    this.selectedIndex = findDeepLinkedStatusEffect(this.records, initialUrl);
    this.abilityOptions = context.session
      .collection('abilities')
      .map(recordName)
      .filter((name): name is string => name !== null)
      .sort((left, right) => left.localeCompare(right))
      .map((value) => ({ value }));
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', {
      className: 'editor-sidebar',
      attributes: { 'aria-label': 'Status effects' },
    });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Name or description';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      field('Search status effects', 'status-effect-search', search),
      button(
        '+ New Status Effect',
        () => this.createRecord(),
        'button--primary',
      ),
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

  private selected(): StatusEffectRecord | undefined {
    return this.records[this.selectedIndex];
  }

  private commit(nextRecord?: StatusEffectRecord): void {
    if (nextRecord && this.selectedIndex >= 0) {
      this.records[this.selectedIndex] = nextRecord;
    }
    this.context.session.replaceCollection(
      'statusEffects',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
  }

  private setSelection(index: number): void {
    this.selectedIndex = index;
    const name = this.selected()?.name;
    const nextUrl = withStatusEffectSelection(
      new URL(window.location.href),
      name,
    );
    window.history.replaceState(null, '', nextUrl);
    this.renderList();
    this.renderForm();
  }

  private createRecord(): void {
    const existingNames = this.records.map((record) => record.name);
    const name = existingNames.includes('NEW_STATUS_EFFECT')
      ? createUniqueStatusEffectName('NEW_STATUS_EFFECT', existingNames)
      : 'NEW_STATUS_EFFECT';
    this.records.push(createStatusEffectRecord(name));
    this.searchTerm = '';
    const search = this.root.querySelector<HTMLInputElement>(
      '#status-effect-search',
    );
    if (search) {
      search.value = '';
    }
    this.commit();
    this.setSelection(this.records.length - 1);
  }

  private cloneSelected(): void {
    const current = this.selected();
    if (!current) {
      return;
    }
    const clone = cloneStatusEffectRecord(
      current,
      this.records.map((record) => record.name),
    );
    const insertAt = this.selectedIndex + 1;
    this.records.splice(insertAt, 0, clone);
    this.commit();
    this.setSelection(insertAt);
  }

  private deleteSelected(): void {
    const current = this.selected();
    if (
      !current ||
      !window.confirm(
        `Delete status effect "${current.name}"? This is saved when you use Save All.`,
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
    update: (current: StatusEffectRecord) => StatusEffectRecord,
    options: { renderForm?: boolean } = {},
  ): void {
    const current = this.selected();
    if (!current) {
      return;
    }
    const updated = update(current);
    this.commit(updated);
    const nextUrl = withStatusEffectSelection(
      new URL(window.location.href),
      updated.name,
    );
    window.history.replaceState(null, '', nextUrl);
    this.renderList();
    if (options.renderForm) {
      this.renderForm();
    }
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matching = this.records
      .map((record, index) => ({ index, record }))
      .filter(({ record }) =>
        matchesStatusEffectSearch(record, this.searchTerm),
      );
    this.resultCount.textContent = `${matching.length} of ${this.records.length}`;

    if (matching.length === 0) {
      const item = element('li', { className: 'empty-state' });
      item.append(element('span', { text: 'No status effects found.' }));
      this.list.append(item);
      return;
    }

    for (const { index, record } of matching) {
      const item = element('li', { className: 'entity-list__item' });
      const select = element('button', {
        className: 'entity-card',
        attributes: {
          type: 'button',
          'aria-current': String(index === this.selectedIndex),
        },
      });
      const body = element('span', { className: 'entity-card__body' });
      body.append(
        element('span', {
          className: 'entity-card__title',
          text: record.name || '(unnamed status effect)',
        }),
        element('span', {
          className: 'entity-card__subtitle',
          text: record.description || 'No description',
        }),
      );
      select.append(body);
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
          text: 'Select a status effect to edit, or create a new one.',
        }),
      );
      return;
    }

    const form = element('form', {
      className: 'editor-form',
      attributes: { novalidate: '' },
    });
    form.addEventListener('submit', (event) => event.preventDefault());
    form.append(
      this.renderIdentity(current),
      this.renderDuration(current),
      this.renderBonuses(current),
      this.renderCurrentStats(current),
      this.renderResistances(current),
      this.renderActions(current),
    );

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

  private renderIdentity(current: StatusEffectRecord): HTMLFieldSetElement {
    const result = section('Status effect');
    const grid = element('div', { className: 'form-grid' });
    const name = element('input');
    name.type = 'text';
    name.required = true;
    name.value = current.name;
    name.autocomplete = 'off';
    name.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, name: name.value })),
    );
    const duration = numberInput(current.baseDuration);
    duration.min = '0';
    duration.step = '1';
    duration.addEventListener('input', () => {
      if (duration.value !== '') {
        this.updateRecord((record) => ({
          ...record,
          baseDuration: duration.valueAsNumber,
        }));
      }
    });
    grid.append(
      field('Name (ID)', 'status-effect-name', name),
      field(
        'Base duration (turns)',
        'status-effect-base-duration',
        duration,
        'Default duration before stat and feat modifiers.',
      ),
    );
    const description = element('textarea');
    description.rows = 3;
    description.value = current.description;
    description.addEventListener('input', () =>
      this.updateRecord((record) => ({
        ...record,
        description: description.value,
      })),
    );
    result.append(
      grid,
      field('Description', 'status-effect-description', description),
    );
    return result;
  }

  private renderDuration(current: StatusEffectRecord): HTMLFieldSetElement {
    const result = section(
      'Duration scale',
      "Optional extra turns: floor(applier's stat × multiplier).",
    );
    result.append(
      checkbox(
        'status-effect-duration-enabled',
        'Scale duration from an applier stat',
        current.durationScale !== undefined,
        (enabled) =>
          this.updateRecord(
            (record) => {
              if (enabled) {
                return {
                  ...record,
                  durationScale: createDefaultDurationScale(),
                };
              }
              const remaining = structuredClone(record);
              delete remaining.durationScale;
              return remaining;
            },
            { renderForm: true },
          ),
      ),
    );
    if (!current.durationScale) {
      return result;
    }

    const grid = element('div', { className: 'form-grid' });
    const stat = selectInput(STAT_TYPES, current.durationScale.durationStat);
    stat.addEventListener('change', () =>
      this.updateRecord((record) => ({
        ...record,
        durationScale: {
          ...record.durationScale!,
          durationStat: stat.value as StatType,
        },
      })),
    );
    const multiplier = numberInput(current.durationScale.durationStatMult);
    multiplier.addEventListener('input', () => {
      if (multiplier.value !== '') {
        this.updateRecord((record) => ({
          ...record,
          durationScale: {
            ...record.durationScale!,
            durationStatMult: multiplier.valueAsNumber,
          },
        }));
      }
    });
    grid.append(
      field('Stat', 'status-effect-duration-stat', stat),
      field('Stat multiplier', 'status-effect-duration-multiplier', multiplier),
    );
    result.append(grid);
    return result;
  }

  private renderBonuses(current: StatusEffectRecord): HTMLFieldSetElement {
    const result = section(
      'Apply bonuses',
      'Passive stat modifiers while the status is active.',
    );
    result.append(
      checkbox(
        'status-effect-bonuses-enabled',
        'Enable stat bonuses',
        current.applyBonuses !== undefined,
        (enabled) =>
          this.updateRecord(
            (record) => {
              if (enabled) {
                return {
                  ...record,
                  applyBonuses: { STR: 0, MND: 0, CON: 0, AGI: 0, LCK: 0 },
                };
              }
              const remaining = structuredClone(record);
              delete remaining.applyBonuses;
              return remaining;
            },
            { renderForm: true },
          ),
      ),
    );
    if (current.applyBonuses) {
      result.append(
        this.renderStatFields('bonus', current.applyBonuses, [
          'STR',
          'MND',
          'CON',
          'AGI',
          'LCK',
        ]),
      );
    }
    return result;
  }

  private renderCurrentStats(current: StatusEffectRecord): HTMLFieldSetElement {
    const result = section(
      'Apply current-stat change',
      'Passive current-stat changes while the status is active.',
    );
    result.append(
      checkbox(
        'status-effect-current-stats-enabled',
        'Enable current-stat changes',
        current.applyCurrentStatChange !== undefined,
        (enabled) =>
          this.updateRecord(
            (record) => {
              if (enabled) {
                return {
                  ...record,
                  applyCurrentStatChange: { HP: 0, AP: 0, MANA: 0, AC: 0 },
                };
              }
              const remaining = structuredClone(record);
              delete remaining.applyCurrentStatChange;
              return remaining;
            },
            { renderForm: true },
          ),
      ),
    );
    if (current.applyCurrentStatChange) {
      result.append(
        this.renderStatFields('current', current.applyCurrentStatChange, [
          'HP',
          'AP',
          'MANA',
          'AC',
        ]),
      );
    }
    return result;
  }

  private renderStatFields(
    prefix: 'bonus' | 'current',
    values: StatusEffectStats | StatusEffectCurrentStats,
    keys: readonly string[],
  ): HTMLDivElement {
    const grid = element('div', { className: 'form-grid' });
    for (const key of keys) {
      const value = values[key];
      const input = numberInput(typeof value === 'number' ? value : 0);
      input.addEventListener('input', () => {
        if (input.value === '') {
          return;
        }
        this.updateRecord((record) => {
          if (prefix === 'bonus') {
            return {
              ...record,
              applyBonuses: {
                ...record.applyBonuses!,
                [key]: input.valueAsNumber,
              },
            };
          }
          return {
            ...record,
            applyCurrentStatChange: {
              ...record.applyCurrentStatChange!,
              [key]: input.valueAsNumber,
            },
          };
        });
      });
      grid.append(field(key, `status-effect-${prefix}-${key}`, input));
    }
    return grid;
  }

  private renderResistances(current: StatusEffectRecord): HTMLFieldSetElement {
    const result = section('Resistances');
    const list = element('div', { className: 'repeatable-list' });
    const resistances = current.applyResistances ?? [];
    resistances.forEach((resistance, index) =>
      list.append(this.renderResistance(resistance, index)),
    );
    if (resistances.length === 0) {
      list.append(
        element('p', { className: 'muted', text: 'No resistances.' }),
      );
    }
    result.append(
      list,
      button('+ Add Resistance', () => {
        this.updateRecord(
          (record) => ({
            ...record,
            applyResistances: [
              ...(record.applyResistances ?? []),
              { attackType: DAMAGE_TYPES[0], mod: 0 },
            ],
          }),
          { renderForm: true },
        );
      }),
    );
    return result;
  }

  private renderResistance(
    resistance: StatusEffectResistance,
    index: number,
  ): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    const header = element('div', { className: 'repeatable-item__header' });
    header.append(
      element('h3', {
        className: 'repeatable-item__title',
        text: `Resistance ${index + 1}`,
      }),
      button(
        'Remove',
        () =>
          this.updateRecord(
            (record) => ({
              ...record,
              applyResistances: (record.applyResistances ?? []).filter(
                (_, itemIndex) => itemIndex !== index,
              ),
            }),
            { renderForm: true },
          ),
        'button--danger button--small',
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const damage = selectInput(DAMAGE_TYPES, resistance.attackType);
    damage.addEventListener('change', () =>
      this.updateResistance(index, (value) => ({
        ...value,
        attackType: damage.value as DamageType,
      })),
    );
    const modifier = numberInput(resistance.mod);
    modifier.addEventListener('input', () => {
      if (modifier.value !== '') {
        this.updateResistance(index, (value) => ({
          ...value,
          mod: modifier.valueAsNumber,
        }));
      }
    });
    grid.append(
      field('Damage type', `resistance-${index}-type`, damage),
      field('Modifier', `resistance-${index}-modifier`, modifier),
    );
    item.append(header, grid);
    return item;
  }

  private updateResistance(
    index: number,
    update: (value: StatusEffectResistance) => StatusEffectResistance,
  ): void {
    this.updateRecord((record) => ({
      ...record,
      applyResistances: (record.applyResistances ?? []).map(
        (value, itemIndex) => (itemIndex === index ? update(value) : value),
      ),
    }));
  }

  private renderActions(current: StatusEffectRecord): HTMLFieldSetElement {
    const result = section(
      'Triggered abilities',
      'Each action invokes an ability when one of its events occurs.',
    );
    const list = element('div', { className: 'repeatable-list' });
    const actions = current.actions ?? [];
    actions.forEach((action, index) =>
      list.append(this.renderAction(action, index)),
    );
    if (actions.length === 0) {
      list.append(
        element('p', { className: 'muted', text: 'No triggered abilities.' }),
      );
    }
    result.append(
      list,
      button('+ Add Triggered Ability', () => {
        this.updateRecord(
          (record) => ({
            ...record,
            actions: [
              ...(record.actions ?? []),
              createDefaultStatusAction(this.abilityOptions[0]?.value),
            ],
          }),
          { renderForm: true },
        );
      }),
    );
    return result;
  }

  private renderAction(
    action: StatusEffectAction,
    index: number,
  ): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    const header = element('div', { className: 'repeatable-item__header' });
    header.append(
      element('h3', {
        className: 'repeatable-item__title',
        text: `Triggered ability ${index + 1}`,
      }),
      button(
        'Remove',
        () =>
          this.updateRecord(
            (record) => ({
              ...record,
              actions: (record.actions ?? []).filter(
                (_, actionIndex) => actionIndex !== index,
              ),
            }),
            { renderForm: true },
          ),
        'button--danger button--small',
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const target = selectInput(
      STATUS_ACTION_TARGET_TYPES,
      action.statusActionTargetType,
    );
    target.addEventListener('change', () =>
      this.updateAction(index, (value) => ({
        ...value,
        statusActionTargetType: target.value as StatusActionTargetType,
      })),
    );
    const ability = selectInput(
      [{ value: '', label: '(select ability)' }, ...this.abilityOptions],
      action.abilityName,
    );
    ability.addEventListener('change', () =>
      this.updateAction(index, (value) => ({
        ...value,
        abilityName: ability.value,
      })),
    );
    grid.append(
      field('Target', `status-action-${index}-target`, target),
      field('Ability', `status-action-${index}-ability`, ability),
    );
    item.append(header, grid, this.renderEvents(action, index));
    return item;
  }

  private updateAction(
    index: number,
    update: (value: StatusEffectAction) => StatusEffectAction,
    renderForm = false,
  ): void {
    this.updateRecord(
      (record) => ({
        ...record,
        actions: (record.actions ?? []).map((value, actionIndex) =>
          actionIndex === index ? update(value) : value,
        ),
      }),
      { renderForm },
    );
  }

  private renderEvents(
    action: StatusEffectAction,
    actionIndex: number,
  ): HTMLElement {
    const container = element('section', { className: 'form-stack' });
    container.append(element('h4', { text: 'Events' }));
    const list = element('div', { className: 'repeatable-list' });
    action.events.forEach((event, eventIndex) =>
      list.append(this.renderEvent(event, actionIndex, eventIndex)),
    );
    if (action.events.length === 0) {
      list.append(element('p', { className: 'muted', text: 'No events.' }));
    }
    container.append(
      list,
      button('+ Add Event', () => {
        this.updateAction(
          actionIndex,
          (value) => ({
            ...value,
            events: [
              ...value.events,
              {
                type: STATUS_EVENT_TYPES[0],
                condition: STATUS_EFFECT_CONDITIONS[0],
              },
            ],
          }),
          true,
        );
      }),
    );
    return container;
  }

  private renderEvent(
    statusEvent: StatusEffectEvent,
    actionIndex: number,
    eventIndex: number,
  ): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    const header = element('div', { className: 'repeatable-item__header' });
    header.append(
      element('h5', {
        className: 'repeatable-item__title',
        text: `Event ${eventIndex + 1}`,
      }),
      button(
        'Remove',
        () =>
          this.updateAction(
            actionIndex,
            (action) => ({
              ...action,
              events: action.events.filter((_, index) => index !== eventIndex),
            }),
            true,
          ),
        'button--danger button--small',
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const type = selectInput(STATUS_EVENT_TYPES, statusEvent.type);
    type.addEventListener('change', () =>
      this.updateEvent(actionIndex, eventIndex, (value) => ({
        ...value,
        type: type.value as StatusEventType,
      })),
    );
    const condition = selectInput(
      STATUS_EFFECT_CONDITIONS,
      statusEvent.condition,
    );
    condition.addEventListener('change', () =>
      this.updateEvent(actionIndex, eventIndex, (value) => ({
        ...value,
        condition: condition.value as StatusEffectCondition,
      })),
    );
    grid.append(
      field(
        'Event',
        `status-action-${actionIndex}-event-${eventIndex}-type`,
        type,
      ),
      field(
        'Condition',
        `status-action-${actionIndex}-event-${eventIndex}-condition`,
        condition,
      ),
    );
    item.append(header, grid);
    return item;
  }

  private updateEvent(
    actionIndex: number,
    eventIndex: number,
    update: (value: StatusEffectEvent) => StatusEffectEvent,
  ): void {
    this.updateAction(actionIndex, (action) => ({
      ...action,
      events: action.events.map((value, index) =>
        index === eventIndex ? update(value) : value,
      ),
    }));
  }
}
