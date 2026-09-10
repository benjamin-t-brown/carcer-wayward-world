import type { JsonArray, JsonObject } from '../../core/database/index.js';
import {
  CURRENT_STAT_TYPES,
  DICE_TYPES,
  STAT_TYPES,
  type AbilityAttackDamage,
  type AbilityRestore,
} from '../../core/domain/abilities/index.js';
import {
  ITEM_TYPES,
  ITEM_USABILITIES,
  RUNE_TYPES,
  changeItemType,
  changeItemUsability,
  cloneItemRecord,
  createDefaultDamageOverride,
  createDefaultItemRecord,
  createUniqueItemName,
  isItemUsable,
  isWeaponItemType,
  itemStatusEffectName,
  parseItemCollection,
  runeIcon,
  type ItemRecord,
  type ItemStatusEffectRef,
  type ItemType,
  type ItemUsability,
  type RuneType,
} from '../../core/domain/items/index.js';
import { element } from '../../core/ui/dom.js';
import type { DatabasePageContext } from '../../core/ui/index.js';
import {
  findDeepLinkedItem,
  matchesItemSearch,
  withItemSelection,
} from './editorModel.js';

type SelectOption = { label?: string; value: string };
type AbilitySummary = {
  attacks: AbilityAttackDamage[];
  label: string;
  name: string;
  restores: AbilityRestore[];
};

function objectValue(value: unknown): JsonObject | undefined {
  return typeof value === 'object' && value !== null && !Array.isArray(value)
    ? (value as JsonObject)
    : undefined;
}

function namedOptions(
  values: JsonArray,
  idKey: 'id' | 'name' = 'name',
): SelectOption[] {
  return values
    .map(objectValue)
    .flatMap((value): SelectOption[] => {
      if (!value) return [];
      const id = value[idKey];
      const label = value.label ?? value.title;
      return typeof id === 'string' && id
        ? [
            {
              value: id,
              label:
                typeof label === 'string' && label ? `${label} — ${id}` : id,
            },
          ]
        : [];
    })
    .sort((left, right) => left.value.localeCompare(right.value));
}

function abilitySummaries(values: JsonArray): AbilitySummary[] {
  return values.flatMap((raw): AbilitySummary[] => {
    const value = objectValue(raw);
    if (!value || typeof value.name !== 'string') return [];
    const attacks = Array.isArray(value.attacks)
      ? value.attacks.flatMap((entry) => {
          const attack = objectValue(entry);
          if (!attack) return [];
          const damage = objectValue(attack.dmg);
          return [
            damage
              ? (structuredClone(damage) as AbilityAttackDamage)
              : createDefaultDamageOverride(),
          ];
        })
      : [];
    const restores = Array.isArray(value.restores)
      ? value.restores.flatMap((entry) => {
          const restore = objectValue(entry);
          return restore ? [structuredClone(restore) as AbilityRestore] : [];
        })
      : [];
    return [
      {
        name: value.name,
        label: typeof value.label === 'string' ? value.label : '',
        attacks,
        restores,
      },
    ];
  });
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

function textInput(value: string): HTMLInputElement {
  const input = element('input');
  input.type = 'text';
  input.value = value;
  input.autocomplete = 'off';
  return input;
}

function numberInput(value: number, step = '1'): HTMLInputElement {
  const input = element('input');
  input.type = 'number';
  input.value = String(value);
  input.step = step;
  return input;
}

function selectInput(
  values: readonly (string | SelectOption)[],
  currentValue: string,
  includeEmpty = false,
): HTMLSelectElement {
  const select = element('select');
  const options: SelectOption[] = values.map((value) =>
    typeof value === 'string' ? { value } : value,
  );
  if (includeEmpty) options.unshift({ value: '', label: '(none)' });
  if (
    currentValue &&
    !options.some((option) => option.value === currentValue)
  ) {
    options.unshift({
      value: currentValue,
      label: `${currentValue} (unrecognized)`,
    });
  }
  options.forEach(({ value, label }) => {
    const option = element('option', { text: label ?? value });
    option.value = value;
    select.append(option);
  });
  select.value = currentValue;
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

export class ItemsEditor {
  private records: ItemRecord[];
  private selectedIndex: number;
  private searchTerm = '';
  private readonly abilities: AbilitySummary[];
  private readonly abilityOptions: SelectOption[];
  private readonly statusOptions: SelectOption[];
  private readonly eventOptions: SelectOption[];
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly resultCount = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseItemCollection(context.session.collection('items'));
    this.selectedIndex = findDeepLinkedItem(this.records, initialUrl);
    const rawAbilities = context.session.collection('abilities');
    this.abilities = abilitySummaries(rawAbilities);
    this.abilityOptions = namedOptions(rawAbilities);
    this.statusOptions = namedOptions(
      context.session.collection('statusEffects'),
    );
    this.eventOptions = namedOptions(
      context.session.collection('specialEvents'),
      'id',
    );
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', {
      className: 'editor-sidebar',
      attributes: { 'aria-label': 'Items' },
    });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Name, label, description, or type';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      field('Search items', 'item-search', search),
      button('+ New Item', () => this.createRecord(), 'button--primary'),
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

  private selected(): ItemRecord | undefined {
    return this.records[this.selectedIndex];
  }

  private commit(next?: ItemRecord): void {
    if (next && this.selectedIndex >= 0)
      this.records[this.selectedIndex] = next;
    this.context.session.replaceCollection('items', this.records as JsonArray);
    this.context.notifyChanged();
  }

  private setSelection(index: number): void {
    this.selectedIndex = index;
    window.history.replaceState(
      null,
      '',
      withItemSelection(new URL(window.location.href), this.selected()?.name),
    );
    this.renderList();
    this.renderForm();
  }

  private createRecord(): void {
    const names = this.records.map((record) => record.name);
    const name = names.includes('NEW_ITEM')
      ? createUniqueItemName('NEW_ITEM', names)
      : 'NEW_ITEM';
    this.records.push(createDefaultItemRecord(name));
    this.searchTerm = '';
    const search = this.root.querySelector<HTMLInputElement>('#item-search');
    if (search) search.value = '';
    this.commit();
    this.setSelection(this.records.length - 1);
  }

  private cloneSelected(): void {
    const current = this.selected();
    if (!current) return;
    const clone = cloneItemRecord(
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
        `Delete item "${current.name}"? This is saved when you use Save All.`,
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
    update: (current: ItemRecord) => ItemRecord,
    renderForm = false,
  ): void {
    const current = this.selected();
    if (!current) return;
    const updated = update(current);
    this.commit(updated);
    window.history.replaceState(
      null,
      '',
      withItemSelection(new URL(window.location.href), updated.name),
    );
    this.renderList();
    if (renderForm) this.renderForm();
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matching = this.records
      .map((record, index) => ({ index, record }))
      .filter(({ record }) => matchesItemSearch(record, this.searchTerm));
    this.resultCount.textContent = `${matching.length} of ${this.records.length}`;
    if (matching.length === 0) {
      this.list.append(
        element('li', { className: 'empty-state', text: 'No items found.' }),
      );
      return;
    }
    matching.forEach(({ index, record }) => {
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
          text: record.label || record.name || '(unnamed item)',
        }),
        element('span', {
          className: 'entity-card__subtitle',
          text: `${record.name} · ${record.itemType}`,
        }),
      );
      select.append(body);
      select.addEventListener('click', () => this.setSelection(index));
      item.append(select);
      this.list.append(item);
    });
  }

  private renderForm(): void {
    this.main.replaceChildren();
    const current = this.selected();
    if (!current) {
      this.main.append(
        element('div', {
          className: 'empty-state',
          text: 'Select an item to edit, or create a new one.',
        }),
      );
      return;
    }
    const form = element('form', {
      className: 'editor-form',
      attributes: { novalidate: '' },
    });
    form.addEventListener('submit', (event) => event.preventDefault());
    form.append(this.renderIdentity(current));
    if (current.itemType === 'RUNE') form.append(this.renderRune(current));
    form.append(this.renderFlags(current), this.renderUse(current));
    if (isWeaponItemType(current.itemType)) {
      form.append(this.renderWeapon(current));
    }
    form.append(this.renderStatuses(current));
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

  private renderIdentity(current: ItemRecord): HTMLFieldSetElement {
    const result = section('Item');
    const grid = element('div', { className: 'form-grid' });
    const itemType = selectInput(ITEM_TYPES, current.itemType);
    itemType.addEventListener('change', () =>
      this.updateRecord(
        (record) => changeItemType(record, itemType.value as ItemType),
        true,
      ),
    );
    const name = textInput(current.name);
    name.required = true;
    name.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, name: name.value })),
    );
    const label = textInput(current.label);
    label.required = true;
    label.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, label: label.value })),
    );
    const icon = textInput(current.icon);
    icon.required = true;
    icon.addEventListener('input', () =>
      this.updateRecord((record) => ({ ...record, icon: icon.value })),
    );
    const weight = numberInput(current.weight);
    weight.addEventListener('input', () => {
      if (weight.value !== '') {
        this.updateRecord((record) => ({
          ...record,
          weight: weight.valueAsNumber,
        }));
      }
    });
    const value = numberInput(current.value);
    value.addEventListener('input', () => {
      if (value.value !== '') {
        this.updateRecord((record) => ({
          ...record,
          value: value.valueAsNumber,
        }));
      }
    });
    grid.append(
      field('Item type', 'item-type', itemType),
      field('Name (ID)', 'item-name', name),
      field('Label', 'item-label', label),
      field('Icon sprite', 'item-icon', icon),
      field('Weight', 'item-weight', weight),
      field('Value', 'item-value', value),
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
    result.append(grid, field('Description', 'item-description', description));
    return result;
  }

  private renderRune(current: ItemRecord): HTMLFieldSetElement {
    const result = section('Rune', 'Rune type is required for RUNE items.');
    const runeType = selectInput(RUNE_TYPES, current.runeType ?? 'HEAT');
    runeType.addEventListener('change', () =>
      this.updateRecord((record) => {
        const next = structuredClone(record);
        const previousIcon = next.runeType ? runeIcon(next.runeType) : '';
        next.runeType = runeType.value as RuneType;
        if (!next.icon || next.icon === previousIcon) {
          next.icon = runeIcon(next.runeType);
        }
        return next;
      }),
    );
    result.append(field('Rune type', 'item-rune-type', runeType));
    return result;
  }

  private renderFlags(current: ItemRecord): HTMLFieldSetElement {
    const result = section('Inventory behavior');
    result.append(
      checkbox(
        'item-stackable',
        'Stackable',
        current.stackable === true,
        (checked) =>
          this.updateRecord((record) => ({ ...record, stackable: checked })),
      ),
      checkbox(
        'item-indestructable',
        'Indestructable (cannot be dropped or destroyed)',
        current.indestructable === true,
        (checked) =>
          this.updateRecord((record) => ({
            ...record,
            indestructable: checked,
          })),
      ),
    );
    return result;
  }

  private renderUse(current: ItemRecord): HTMLFieldSetElement {
    const result = section(
      'Use behavior',
      'A usable item can invoke either an ability or a special event.',
    );
    const usability = selectInput(
      ITEM_USABILITIES,
      current.itemUsability ?? 'NOT_USABLE',
    );
    usability.addEventListener('change', () =>
      this.updateRecord(
        (record) =>
          changeItemUsability(record, usability.value as ItemUsability),
        true,
      ),
    );
    result.append(field('Where usable', 'item-usability', usability));
    if (!isItemUsable(current.itemUsability)) return result;

    const behavior = selectInput(
      [
        { value: 'ABILITY', label: 'Ability' },
        { value: 'SPECIAL_EVENT', label: 'Special event' },
      ],
      current.useSpecialEvent !== undefined ? 'SPECIAL_EVENT' : 'ABILITY',
    );
    behavior.addEventListener('change', () =>
      this.updateRecord((record) => {
        const next = structuredClone(record);
        if (behavior.value === 'SPECIAL_EVENT') {
          delete next.useAbility;
          next.useSpecialEvent ??= '';
        } else {
          delete next.useSpecialEvent;
          next.useAbility ??= { abilityName: '' };
        }
        return next;
      }, true),
    );
    result.append(field('Action', 'item-use-kind', behavior));
    if (current.useSpecialEvent !== undefined) {
      const event = selectInput(
        this.eventOptions,
        current.useSpecialEvent,
        true,
      );
      event.addEventListener('change', () =>
        this.updateRecord((record) => ({
          ...record,
          useSpecialEvent: event.value,
        })),
      );
      result.append(field('Special event', 'item-use-event', event));
      return result;
    }

    const abilityName = current.useAbility?.abilityName ?? '';
    const ability = selectInput(this.abilityOptions, abilityName, true);
    ability.addEventListener('change', () =>
      this.updateRecord((record) => {
        const next = structuredClone(record);
        const base = this.abilities.find(
          (entry) => entry.name === ability.value,
        );
        next.useAbility = {
          ...(next.useAbility ?? {}),
          abilityName: ability.value,
          ...(base?.attacks.length
            ? { dmgOverrides: structuredClone(base.attacks) }
            : {}),
          ...(base?.restores.length
            ? { restoreOverrides: structuredClone(base.restores) }
            : {}),
        };
        return next;
      }, true),
    );
    result.append(field('Use ability', 'item-use-ability', ability));
    const selectedAbility = this.abilities.find(
      (entry) => entry.name === abilityName,
    );
    if (selectedAbility?.attacks.length) {
      result.append(
        this.renderDamageOverrides(
          'use',
          current.useAbility?.dmgOverrides,
          selectedAbility.attacks,
          (overrides) =>
            this.updateRecord((record) => ({
              ...record,
              useAbility: { ...record.useAbility!, dmgOverrides: overrides },
            })),
        ),
      );
    }
    if (selectedAbility?.restores.length) {
      result.append(
        this.renderRestoreOverrides(
          current.useAbility?.restoreOverrides,
          selectedAbility.restores,
        ),
      );
    }
    return result;
  }

  private renderWeapon(current: ItemRecord): HTMLFieldSetElement {
    const result = section(
      'Weapon',
      'Damage overrides correspond by index to attacks on the selected ability.',
    );
    const abilityName = current.weapon?.abilityName ?? '';
    const ability = selectInput(this.abilityOptions, abilityName, true);
    ability.addEventListener('change', () =>
      this.updateRecord((record) => {
        const next = structuredClone(record);
        const base = this.abilities.find(
          (entry) => entry.name === ability.value,
        );
        next.weapon = {
          ...(next.weapon ?? {}),
          abilityName: ability.value,
          ...(base?.attacks.length
            ? { dmgOverrides: structuredClone(base.attacks) }
            : {}),
        };
        return next;
      }, true),
    );
    result.append(field('Base ability', 'item-weapon-ability', ability));
    const selectedAbility = this.abilities.find(
      (entry) => entry.name === abilityName,
    );
    if (selectedAbility?.attacks.length) {
      result.append(
        this.renderDamageOverrides(
          'weapon',
          current.weapon?.dmgOverrides,
          selectedAbility.attacks,
          (overrides) =>
            this.updateRecord((record) => ({
              ...record,
              weapon: { ...record.weapon!, dmgOverrides: overrides },
            })),
        ),
      );
    }
    return result;
  }

  private renderDamageOverrides(
    prefix: 'use' | 'weapon',
    stored: AbilityAttackDamage[] | undefined,
    defaults: AbilityAttackDamage[],
    commit: (overrides: AbilityAttackDamage[]) => void,
  ): HTMLDivElement {
    const list = element('div', { className: 'repeatable-list' });
    defaults.forEach((fallback, index) => {
      const value = stored?.[index] ?? fallback;
      const item = element('div', { className: 'repeatable-item' });
      item.append(
        element('h3', {
          className: 'repeatable-item__title',
          text: `Attack override ${index + 1}`,
        }),
      );
      const update = (nextValue: AbilityAttackDamage) => {
        const overrides = defaults.map((entry, itemIndex) =>
          structuredClone(stored?.[itemIndex] ?? entry),
        );
        overrides[index] = nextValue;
        commit(overrides);
      };
      item.append(
        this.damageFields(`${prefix}-damage-${index}`, value, update),
      );
      list.append(item);
    });
    return list;
  }

  private damageFields(
    prefix: string,
    value: AbilityAttackDamage,
    update: (value: AbilityAttackDamage) => void,
  ): HTMLDivElement {
    const grid = element('div', { className: 'form-grid' });
    const dice = textInput(value.dmgDice.join(', '));
    dice.addEventListener('change', () =>
      update({
        ...value,
        dmgDice: dice.value
          .split(',')
          .map((entry) => entry.trim())
          .filter((entry) =>
            DICE_TYPES.includes(entry as never),
          ) as AbilityAttackDamage['dmgDice'],
      }),
    );
    const bonus = numberInput(value.dmgBonus);
    bonus.addEventListener('input', () => {
      if (bonus.value !== '')
        update({ ...value, dmgBonus: bonus.valueAsNumber });
    });
    const stat = selectInput(STAT_TYPES, value.dmgStat);
    stat.addEventListener('change', () =>
      update({
        ...value,
        dmgStat: stat.value as AbilityAttackDamage['dmgStat'],
      }),
    );
    const multiplier = numberInput(value.dmgStatMult, 'any');
    multiplier.addEventListener('input', () => {
      if (multiplier.value !== '') {
        update({ ...value, dmgStatMult: multiplier.valueAsNumber });
      }
    });
    const attackBonus = numberInput(value.attackBonus);
    attackBonus.addEventListener('input', () => {
      if (attackBonus.value !== '') {
        update({ ...value, attackBonus: attackBonus.valueAsNumber });
      }
    });
    grid.append(
      field('Dice (comma separated)', `${prefix}-dice`, dice),
      field('Damage bonus', `${prefix}-bonus`, bonus),
      field('Damage stat', `${prefix}-stat`, stat),
      field('Stat multiplier', `${prefix}-multiplier`, multiplier),
      field('Attack bonus', `${prefix}-attack-bonus`, attackBonus),
    );
    return grid;
  }

  private renderRestoreOverrides(
    stored: AbilityRestore[] | undefined,
    defaults: AbilityRestore[],
  ): HTMLDivElement {
    const list = element('div', { className: 'repeatable-list' });
    defaults.forEach((fallback, index) => {
      const value = stored?.[index] ?? fallback;
      const item = element('div', { className: 'repeatable-item' });
      item.append(
        element('h3', {
          className: 'repeatable-item__title',
          text: `Restore override ${index + 1}`,
        }),
      );
      const update = (nextValue: AbilityRestore) => {
        const overrides = defaults.map((entry, itemIndex) =>
          structuredClone(stored?.[itemIndex] ?? entry),
        );
        overrides[index] = nextValue;
        this.updateRecord((record) => ({
          ...record,
          useAbility: { ...record.useAbility!, restoreOverrides: overrides },
        }));
      };
      const grid = element('div', { className: 'form-grid' });
      const which = selectInput(CURRENT_STAT_TYPES, value.restoreWhich);
      which.addEventListener('change', () =>
        update({
          ...value,
          restoreWhich: which.value as AbilityRestore['restoreWhich'],
        }),
      );
      const dice = textInput(value.restoreDice.join(', '));
      dice.addEventListener('change', () =>
        update({
          ...value,
          restoreDice: dice.value
            .split(',')
            .map((entry) => entry.trim())
            .filter((entry) =>
              DICE_TYPES.includes(entry as never),
            ) as AbilityRestore['restoreDice'],
        }),
      );
      const bonus = numberInput(value.restoreBonus);
      bonus.addEventListener('input', () => {
        if (bonus.value !== '') {
          update({ ...value, restoreBonus: bonus.valueAsNumber });
        }
      });
      const stat = selectInput(STAT_TYPES, value.restoreStat);
      stat.addEventListener('change', () =>
        update({
          ...value,
          restoreStat: stat.value as AbilityRestore['restoreStat'],
        }),
      );
      const multiplier = numberInput(value.restoreStatMult);
      multiplier.addEventListener('input', () => {
        if (multiplier.value !== '') {
          update({ ...value, restoreStatMult: multiplier.valueAsNumber });
        }
      });
      grid.append(
        field('Restore stat', `use-restore-${index}-which`, which),
        field('Dice (comma separated)', `use-restore-${index}-dice`, dice),
        field('Restore bonus', `use-restore-${index}-bonus`, bonus),
        field('Scaling stat', `use-restore-${index}-stat`, stat),
        field('Stat multiplier', `use-restore-${index}-multiplier`, multiplier),
      );
      item.append(grid);
      list.append(item);
    });
    return list;
  }

  private renderStatuses(current: ItemRecord): HTMLFieldSetElement {
    const result = section(
      'Equipped status effects',
      'Name strings and object references are both retained in their original storage shape.',
    );
    const list = element('div', { className: 'repeatable-list' });
    const statuses = current.statusEffects ?? [];
    statuses.forEach((reference, index) => {
      const item = element('div', { className: 'repeatable-item' });
      const header = element('div', { className: 'repeatable-item__header' });
      header.append(
        element('h3', {
          className: 'repeatable-item__title',
          text: `Status effect ${index + 1}`,
        }),
        button(
          'Remove',
          () =>
            this.updateRecord(
              (record) => ({
                ...record,
                statusEffects: (record.statusEffects ?? []).filter(
                  (_, itemIndex) => itemIndex !== index,
                ),
              }),
              true,
            ),
          'button--danger button--small',
        ),
      );
      const select = selectInput(
        this.statusOptions,
        itemStatusEffectName(reference),
        true,
      );
      select.addEventListener('change', () =>
        this.updateRecord((record) => ({
          ...record,
          statusEffects: (record.statusEffects ?? []).map((entry, itemIndex) =>
            itemIndex === index
              ? this.renameStatusReference(entry, select.value)
              : entry,
          ),
        })),
      );
      item.append(
        header,
        field('Status effect', `item-status-${index}`, select),
      );
      list.append(item);
    });
    if (statuses.length === 0) {
      list.append(
        element('p', { className: 'muted', text: 'No status effects.' }),
      );
    }
    result.append(
      list,
      button('+ Add Status Effect', () =>
        this.updateRecord(
          (record) => ({
            ...record,
            statusEffects: [
              ...(record.statusEffects ?? []),
              this.statusOptions[0]?.value ?? '',
            ],
          }),
          true,
        ),
      ),
    );
    return result;
  }

  private renameStatusReference(
    reference: ItemStatusEffectRef,
    name: string,
  ): ItemStatusEffectRef {
    return typeof reference === 'string' ? name : { ...reference, name };
  }
}
