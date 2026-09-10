import type { JsonArray } from '../../core/database/index.js';
import {
  ABILITY_COST_TYPES,
  ABILITY_TYPES,
  ATTACK_CLASSES,
  CURRENT_STAT_TYPES,
  DAMAGE_TYPES,
  DICE_TYPES,
  PROJECTILE_PATHS,
  PROJECTILE_TYPES,
  STAT_TYPES,
  TARGET_ALLEGIANCE_TYPES,
  TARGET_SELECT_TYPES,
  cloneAbilityRecord,
  createDefaultAbilityRecord,
  createDefaultAbilitySave,
  createDefaultAttackDamage,
  createUniqueAbilityName,
  parseAbilityCollection,
  type AbilityAttack,
  type AbilityDamage,
  type AbilityDepiction,
  type AbilityRecord,
  type AbilityRestore,
  type AbilitySave,
  type AbilityStatus,
  type AbilityTargetSelect,
  type AbilityType,
  type AbilityCostType,
  type AttackClass,
  type CurrentStatType,
  type DamageType,
  type DieType,
  type ProjectilePath,
  type ProjectileType,
  type StatType,
  type TargetAllegianceType,
  type TargetSelectType,
} from '../../core/domain/abilities/index.js';
import { element } from '../../core/ui/dom.js';
import {
  createEntitySpritePreview,
  createMediaPickerField,
  type DatabasePageContext,
} from '../../core/ui/index.js';
import {
  findDeepLinkedAbility,
  matchesAbilitySearch,
  withAbilitySelection,
} from './editorModel.js';

type SelectOption = { label?: string; value: string };

function recordName(value: unknown): string | null {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return null;
  }
  const name = (value as Record<string, unknown>).name;
  return typeof name === 'string' && name.trim() ? name : null;
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

function numberInput(value: number): HTMLInputElement {
  const input = element('input');
  input.type = 'number';
  input.value = String(value);
  input.step = '1';
  return input;
}

function selectInput(
  values: readonly (string | SelectOption)[],
  currentValue: string,
): HTMLSelectElement {
  const select = element('select');
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

function itemHeader(title: string, onRemove: () => void): HTMLDivElement {
  const header = element('div', { className: 'repeatable-item__header' });
  header.append(
    element('h3', { className: 'repeatable-item__title', text: title }),
    button('Remove', onRemove, 'button--danger button--small'),
  );
  return header;
}

export class AbilitiesEditor {
  private records: AbilityRecord[];
  private selectedIndex: number;
  private searchTerm = '';
  private readonly statusOptions: SelectOption[];
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly resultCount = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseAbilityCollection(
      context.session.collection('abilities'),
    );
    this.selectedIndex = findDeepLinkedAbility(this.records, initialUrl);
    this.statusOptions = context.session
      .collection('statusEffects')
      .map(recordName)
      .filter((name): name is string => name !== null)
      .sort((left, right) => left.localeCompare(right))
      .map((value) => ({ value }));
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', {
      className: 'editor-sidebar',
      attributes: { 'aria-label': 'Abilities' },
    });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Name, label, or description';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      field('Search abilities', 'ability-search', search),
      button('+ New Ability', () => this.createRecord(), 'button--primary'),
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

  private selected(): AbilityRecord | undefined {
    return this.records[this.selectedIndex];
  }

  private commit(next?: AbilityRecord): void {
    if (next && this.selectedIndex >= 0)
      this.records[this.selectedIndex] = next;
    this.context.session.replaceCollection(
      'abilities',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
  }

  private setSelection(index: number): void {
    this.selectedIndex = index;
    window.history.replaceState(
      null,
      '',
      withAbilitySelection(
        new URL(window.location.href),
        this.selected()?.name,
      ),
    );
    this.renderList();
    this.renderForm();
  }

  private createRecord(): void {
    const names = this.records.map(({ name }) => name);
    const name = names.includes('NEW_ABILITY')
      ? createUniqueAbilityName('NEW_ABILITY', names)
      : 'NEW_ABILITY';
    this.records.push(createDefaultAbilityRecord(name));
    this.searchTerm = '';
    const search = this.root.querySelector<HTMLInputElement>('#ability-search');
    if (search) search.value = '';
    this.commit();
    this.setSelection(this.records.length - 1);
  }

  private cloneSelected(): void {
    const current = this.selected();
    if (!current) return;
    const clone = cloneAbilityRecord(
      current,
      this.records.map(({ name }) => name),
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
        `Delete ability "${current.name}"? References are not changed automatically.`,
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
    update: (record: AbilityRecord) => AbilityRecord,
    renderForm = false,
  ): void {
    const current = this.selected();
    if (!current) return;
    const next = update(current);
    this.commit(next);
    window.history.replaceState(
      null,
      '',
      withAbilitySelection(new URL(window.location.href), next.name),
    );
    this.renderList();
    if (renderForm) this.renderForm();
  }

  private updateListItem<
    K extends 'attacks' | 'statuses' | 'restores' | 'damages',
  >(
    key: K,
    index: number,
    update: (
      value: NonNullable<AbilityRecord[K]>[number],
    ) => NonNullable<AbilityRecord[K]>[number],
  ): void {
    this.updateRecord((record) => ({
      ...record,
      [key]: (record[key] ?? []).map((value, itemIndex) =>
        itemIndex === index ? update(value as never) : value,
      ),
    }));
  }

  private removeListItem(
    key: 'attacks' | 'statuses' | 'restores' | 'damages',
    index: number,
  ): void {
    this.updateRecord(
      (record) => ({
        ...record,
        [key]: (record[key] ?? []).filter(
          (_, itemIndex) => itemIndex !== index,
        ),
      }),
      true,
    );
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matching = this.records
      .map((record, index) => ({ record, index }))
      .filter(({ record }) => matchesAbilitySearch(record, this.searchTerm));
    this.resultCount.textContent = `${matching.length} of ${this.records.length}`;
    if (matching.length === 0) {
      this.list.append(
        element('li', {
          className: 'empty-state',
          text: 'No abilities found.',
        }),
      );
      return;
    }
    matching.forEach(({ record, index }) => {
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
          text: record.label || record.name || '(unnamed ability)',
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
    });
  }

  private renderForm(): void {
    this.main.replaceChildren();
    const current = this.selected();
    if (!current) {
      this.main.append(
        element('div', {
          className: 'empty-state',
          text: 'Select an ability to edit, or create a new one.',
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
      this.renderTarget(current),
      this.renderDepiction(current),
      this.renderAttacks(current),
      this.renderStatuses(current),
      this.renderRestores(current),
      this.renderDamages(current),
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

  private renderIdentity(current: AbilityRecord): HTMLFieldSetElement {
    const result = section('Ability');
    const grid = element('div', { className: 'form-grid' });
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
    const type = selectInput(ABILITY_TYPES, current.type);
    type.addEventListener('change', () =>
      this.updateRecord((record) => ({
        ...record,
        type: type.value as AbilityType,
      })),
    );
    const apCost = numberInput(current.apCost);
    apCost.min = '0';
    apCost.addEventListener('input', () => {
      if (apCost.value !== '') {
        this.updateRecord((record) => ({
          ...record,
          apCost: apCost.valueAsNumber,
        }));
      }
    });
    const costType = selectInput(ABILITY_COST_TYPES, current.costType);
    costType.addEventListener('change', () =>
      this.updateRecord((record) => ({
        ...record,
        costType: costType.value as AbilityCostType,
      })),
    );
    const costValue = numberInput(current.costValue);
    costValue.min = '0';
    costValue.addEventListener('input', () => {
      if (costValue.value !== '') {
        this.updateRecord((record) => ({
          ...record,
          costValue: costValue.valueAsNumber,
        }));
      }
    });
    grid.append(
      field('Name (ID)', 'ability-name', name),
      field('Label', 'ability-label', label),
      createMediaPickerField({
        id: 'ability-icon',
        label: 'Icon sprite',
        kind: 'sprite',
        value: current.icon,
        onChange: (value) =>
          this.updateRecord((record) => ({ ...record, icon: value })),
      }),
      field('Type', 'ability-type', type),
      field('AP cost', 'ability-ap-cost', apCost),
      field('Extra cost type', 'ability-cost-type', costType),
      field('Extra cost value', 'ability-cost-value', costValue),
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
      field('Description', 'ability-description', description),
    );
    return result;
  }

  private renderTarget(current: AbilityRecord): HTMLFieldSetElement {
    const result = section('Targeting');
    const grid = element('div', { className: 'form-grid' });
    const target = current.targetSelect;
    const update = (change: Partial<AbilityTargetSelect>) =>
      this.updateRecord((record) => ({
        ...record,
        targetSelect: {
          ...record.targetSelect,
          ...change,
        } as AbilityTargetSelect,
      }));
    const targetType = selectInput(TARGET_SELECT_TYPES, target.targetType);
    targetType.addEventListener('change', () =>
      update({ targetType: targetType.value as TargetSelectType }),
    );
    const allegiance = selectInput(
      TARGET_ALLEGIANCE_TYPES,
      target.allegianceSelectType,
    );
    allegiance.addEventListener('change', () =>
      update({
        allegianceSelectType: allegiance.value as TargetAllegianceType,
      }),
    );
    const numTargets = numberInput(target.numTargetableUnits);
    numTargets.min = '1';
    numTargets.addEventListener('input', () => {
      if (numTargets.value !== '')
        update({ numTargetableUnits: numTargets.valueAsNumber });
    });
    const range = numberInput(target.range);
    range.min = '0';
    range.addEventListener('input', () => {
      if (range.value !== '') update({ range: range.valueAsNumber });
    });
    const zoneX = numberInput(target.zoneSize.x);
    zoneX.min = '1';
    zoneX.addEventListener('input', () => {
      if (zoneX.value !== '') {
        update({ zoneSize: { ...target.zoneSize, x: zoneX.valueAsNumber } });
      }
    });
    const zoneY = numberInput(target.zoneSize.y);
    zoneY.min = '1';
    zoneY.addEventListener('input', () => {
      if (zoneY.value !== '') {
        update({ zoneSize: { ...target.zoneSize, y: zoneY.valueAsNumber } });
      }
    });
    grid.append(
      field('Target type', 'ability-target-type', targetType),
      field('Allegiance', 'ability-target-allegiance', allegiance),
      field('Target count', 'ability-target-count', numTargets),
      field('Range', 'ability-target-range', range),
      field('Zone width', 'ability-zone-width', zoneX),
      field('Zone height', 'ability-zone-height', zoneY),
    );
    result.append(grid);
    return result;
  }

  private renderDepiction(current: AbilityRecord): HTMLFieldSetElement {
    const result = section(
      'Depiction',
      'Animation and sound identifiers are stored as database strings.',
    );
    const grid = element('div', { className: 'form-grid' });
    const depiction = current.depiction;
    const update = (change: Partial<AbilityDepiction>) =>
      this.updateRecord((record) => ({
        ...record,
        depiction: { ...record.depiction, ...change } as AbilityDepiction,
      }));
    const projectileType = selectInput(
      PROJECTILE_TYPES,
      depiction.projectileType ?? 'PROJECTILE_NONE',
    );
    projectileType.addEventListener('change', () =>
      update({ projectileType: projectileType.value as ProjectileType }),
    );
    const projectilePath = selectInput(
      PROJECTILE_PATHS,
      depiction.projectilePath,
    );
    projectilePath.addEventListener('change', () =>
      update({ projectilePath: projectilePath.value as ProjectilePath }),
    );
    grid.append(
      createMediaPickerField({
        id: 'ability-dmg-animation',
        label: 'Damage animation',
        kind: 'animation',
        value: depiction.dmgAnim,
        onChange: (value) => update({ dmgAnim: value }),
      }),
      field('Projectile', 'ability-projectile-type', projectileType),
      field('Projectile path', 'ability-projectile-path', projectilePath),
      createMediaPickerField({
        id: 'ability-start-sound',
        label: 'Start sound',
        kind: 'sound',
        value: depiction.startSound,
        onChange: (value) => update({ startSound: value }),
      }),
      createMediaPickerField({
        id: 'ability-dmg-sound',
        label: 'Damage sound',
        kind: 'sound',
        value: depiction.dmgSound,
        onChange: (value) => update({ dmgSound: value }),
      }),
    );
    result.append(grid);
    return result;
  }

  private renderAttacks(current: AbilityRecord): HTMLFieldSetElement {
    const result = section('Attacks');
    const list = element('div', { className: 'repeatable-list' });
    (current.attacks ?? []).forEach((attack, index) =>
      list.append(this.renderAttack(attack, index)),
    );
    if ((current.attacks ?? []).length === 0) {
      list.append(element('p', { className: 'muted', text: 'No attacks.' }));
    }
    result.append(
      list,
      button('+ Add Attack', () => {
        this.updateRecord(
          (record) => ({
            ...record,
            attacks: [
              ...(record.attacks ?? []),
              {
                attackClass: 'ATTACK_CLASS_MELEE',
                damageType: 'DAMAGE_TYPE_EDGED',
                dmg: createDefaultAttackDamage(),
              },
            ],
          }),
          true,
        );
      }),
    );
    return result;
  }

  private renderAttack(attack: AbilityAttack, index: number): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    item.append(
      itemHeader(`Attack ${index + 1}`, () =>
        this.removeListItem('attacks', index),
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const attackClass = selectInput(ATTACK_CLASSES, attack.attackClass);
    attackClass.addEventListener('change', () =>
      this.updateListItem('attacks', index, (value) => ({
        ...value,
        attackClass: attackClass.value as AttackClass,
      })),
    );
    const damageType = selectInput(
      DAMAGE_TYPES,
      attack.damageType ?? 'DAMAGE_TYPE_EDGED',
    );
    damageType.addEventListener('change', () =>
      this.updateListItem('attacks', index, (value) => ({
        ...value,
        damageType: damageType.value as DamageType,
      })),
    );
    grid.append(
      field('Attack class', `attack-${index}-class`, attackClass),
      field('Damage type', `attack-${index}-type`, damageType),
    );
    item.append(grid);
    item.append(
      checkbox(
        `attack-${index}-damage-enabled`,
        'Roll attack damage',
        attack.dmg !== undefined,
        (enabled) => {
          this.updateListItem('attacks', index, (value) => {
            if (enabled) return { ...value, dmg: createDefaultAttackDamage() };
            const next = structuredClone(value);
            delete next.dmg;
            return next;
          });
          this.renderForm();
        },
      ),
    );
    if (attack.dmg) {
      item.append(
        this.renderAttackDamage(
          attack.dmg,
          `attack-${index}-damage`,
          (damage) =>
            this.updateListItem('attacks', index, (value) => ({
              ...value,
              dmg: damage,
            })),
        ),
      );
    }
    item.append(
      this.renderOptionalSave(attack.save, `attack-${index}-save`, (save) =>
        this.updateListItem('attacks', index, (value) => {
          const next = structuredClone(value);
          if (save) next.save = save;
          else delete next.save;
          return next;
        }),
      ),
    );
    return item;
  }

  private renderAttackDamage(
    damage: NonNullable<AbilityAttack['dmg']>,
    idPrefix: string,
    onChange: (damage: NonNullable<AbilityAttack['dmg']>) => void,
  ): HTMLDivElement {
    const wrapper = element('div', { className: 'form-stack' });
    wrapper.append(
      this.renderDiceList(idPrefix, damage.dmgDice, (dmgDice) =>
        onChange({ ...damage, dmgDice }),
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const bonus = numberInput(damage.dmgBonus);
    bonus.addEventListener('input', () => {
      if (bonus.value !== '')
        onChange({ ...damage, dmgBonus: bonus.valueAsNumber });
    });
    const stat = selectInput(STAT_TYPES, damage.dmgStat);
    stat.addEventListener('change', () =>
      onChange({ ...damage, dmgStat: stat.value as StatType }),
    );
    const multiplier = numberInput(damage.dmgStatMult);
    multiplier.step = 'any';
    multiplier.addEventListener('input', () => {
      if (multiplier.value !== '') {
        onChange({ ...damage, dmgStatMult: multiplier.valueAsNumber });
      }
    });
    const attackBonus = numberInput(damage.attackBonus);
    attackBonus.addEventListener('input', () => {
      if (attackBonus.value !== '') {
        onChange({ ...damage, attackBonus: attackBonus.valueAsNumber });
      }
    });
    grid.append(
      field('Damage bonus', `${idPrefix}-bonus`, bonus),
      field('Damage stat', `${idPrefix}-stat`, stat),
      field('Stat multiplier', `${idPrefix}-multiplier`, multiplier),
      field('Attack bonus', `${idPrefix}-attack-bonus`, attackBonus),
    );
    wrapper.append(grid);
    return wrapper;
  }

  private renderStatuses(current: AbilityRecord): HTMLFieldSetElement {
    const result = section('Status effects');
    const list = element('div', { className: 'repeatable-list' });
    (current.statuses ?? []).forEach((status, index) =>
      list.append(this.renderStatus(status, index)),
    );
    if ((current.statuses ?? []).length === 0) {
      list.append(
        element('p', { className: 'muted', text: 'No status effects.' }),
      );
    }
    result.append(
      list,
      button('+ Add Status Effect', () => {
        this.updateRecord(
          (record) => ({
            ...record,
            statuses: [
              ...(record.statuses ?? []),
              { statusEffect: this.statusOptions[0]?.value ?? '' },
            ],
          }),
          true,
        );
      }),
    );
    return result;
  }

  private renderStatus(status: AbilityStatus, index: number): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    item.append(
      itemHeader(`Status ${index + 1}`, () =>
        this.removeListItem('statuses', index),
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const statusEffect = selectInput(this.statusOptions, status.statusEffect);
    statusEffect.addEventListener('change', () =>
      this.updateListItem('statuses', index, (value) => ({
        ...value,
        statusEffect: statusEffect.value,
      })),
    );
    const baseDuration = numberInput(status.baseDuration ?? 0);
    baseDuration.value =
      status.baseDuration === undefined ? '' : String(status.baseDuration);
    baseDuration.placeholder = 'Template default';
    baseDuration.addEventListener('input', () =>
      this.updateListItem('statuses', index, (value) => {
        const next = structuredClone(value);
        if (baseDuration.value === '') delete next.baseDuration;
        else next.baseDuration = baseDuration.valueAsNumber;
        return next;
      }),
    );
    const durationBonus = numberInput(status.durationBonus ?? 0);
    durationBonus.value =
      status.durationBonus === undefined ? '' : String(status.durationBonus);
    durationBonus.placeholder = 'None';
    durationBonus.addEventListener('input', () =>
      this.updateListItem('statuses', index, (value) => {
        const next = structuredClone(value);
        if (durationBonus.value === '') delete next.durationBonus;
        else next.durationBonus = durationBonus.valueAsNumber;
        return next;
      }),
    );
    grid.append(
      field('Status effect', `status-${index}-effect`, statusEffect),
      field(
        'Base duration override',
        `status-${index}-base-duration`,
        baseDuration,
      ),
      field('Duration bonus', `status-${index}-duration-bonus`, durationBonus),
    );
    item.append(
      grid,
      this.renderOptionalSave(status.save, `status-${index}-save`, (save) =>
        this.updateListItem('statuses', index, (value) => {
          const next = structuredClone(value);
          if (save) next.save = save;
          else delete next.save;
          return next;
        }),
      ),
    );
    return item;
  }

  private renderOptionalSave(
    save: AbilitySave | undefined,
    idPrefix: string,
    onChange: (save: AbilitySave | undefined) => void,
  ): HTMLDivElement {
    const wrapper = element('div', { className: 'form-stack' });
    wrapper.append(
      checkbox(
        `${idPrefix}-enabled`,
        'Allow a saving throw',
        save !== undefined,
        (enabled) => {
          onChange(enabled ? createDefaultAbilitySave() : undefined);
          this.renderForm();
        },
      ),
    );
    if (!save) return wrapper;
    const grid = element('div', { className: 'form-grid' });
    const saveStat = selectInput(STAT_TYPES, save.saveStat);
    saveStat.addEventListener('change', () =>
      onChange({ ...save, saveStat: saveStat.value as StatType }),
    );
    const saveBase = numberInput(save.saveBase);
    saveBase.addEventListener('input', () => {
      if (saveBase.value !== '')
        onChange({ ...save, saveBase: saveBase.valueAsNumber });
    });
    const against = selectInput(STAT_TYPES, save.saveAgainst);
    against.addEventListener('change', () =>
      onChange({ ...save, saveAgainst: against.value as StatType }),
    );
    const againstBase = numberInput(save.saveAgainstBase);
    againstBase.addEventListener('input', () => {
      if (againstBase.value !== '') {
        onChange({ ...save, saveAgainstBase: againstBase.valueAsNumber });
      }
    });
    grid.append(
      field('Save stat', `${idPrefix}-stat`, saveStat),
      field('Save base', `${idPrefix}-base`, saveBase),
      field('Against stat', `${idPrefix}-against`, against),
      field('Against base', `${idPrefix}-against-base`, againstBase),
    );
    wrapper.append(grid);
    return wrapper;
  }

  private renderRestores(current: AbilityRecord): HTMLFieldSetElement {
    const result = section('Restores');
    const list = element('div', { className: 'repeatable-list' });
    (current.restores ?? []).forEach((restore, index) =>
      list.append(this.renderRestore(restore, index)),
    );
    if ((current.restores ?? []).length === 0) {
      list.append(element('p', { className: 'muted', text: 'No restores.' }));
    }
    result.append(
      list,
      button('+ Add Restore', () => {
        this.updateRecord(
          (record) => ({
            ...record,
            restores: [
              ...(record.restores ?? []),
              {
                restoreWhich: 'CURRENT_STAT_HP',
                restoreDice: ['D6'],
                restoreBonus: 0,
                restoreStat: 'STAT_MND',
                restoreStatMult: 1,
              },
            ],
          }),
          true,
        );
      }),
    );
    return result;
  }

  private renderRestore(
    restore: AbilityRestore,
    index: number,
  ): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    item.append(
      itemHeader(`Restore ${index + 1}`, () =>
        this.removeListItem('restores', index),
      ),
      this.renderDiceList(
        `restore-${index}`,
        restore.restoreDice,
        (restoreDice) =>
          this.updateListItem('restores', index, (value) => ({
            ...value,
            restoreDice,
          })),
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const which = selectInput(CURRENT_STAT_TYPES, restore.restoreWhich);
    which.addEventListener('change', () =>
      this.updateListItem('restores', index, (value) => ({
        ...value,
        restoreWhich: which.value as CurrentStatType,
      })),
    );
    const bonus = numberInput(restore.restoreBonus);
    bonus.addEventListener('input', () => {
      if (bonus.value !== '') {
        this.updateListItem('restores', index, (value) => ({
          ...value,
          restoreBonus: bonus.valueAsNumber,
        }));
      }
    });
    const stat = selectInput(STAT_TYPES, restore.restoreStat);
    stat.addEventListener('change', () =>
      this.updateListItem('restores', index, (value) => ({
        ...value,
        restoreStat: stat.value as StatType,
      })),
    );
    const mult = numberInput(restore.restoreStatMult);
    mult.addEventListener('input', () => {
      if (mult.value !== '') {
        this.updateListItem('restores', index, (value) => ({
          ...value,
          restoreStatMult: mult.valueAsNumber,
        }));
      }
    });
    grid.append(
      field('Restore stat', `restore-${index}-which`, which),
      field('Flat bonus', `restore-${index}-bonus`, bonus),
      field('Scaling stat', `restore-${index}-stat`, stat),
      field('Stat multiplier', `restore-${index}-multiplier`, mult),
    );
    item.append(grid);
    return item;
  }

  private renderDamages(current: AbilityRecord): HTMLFieldSetElement {
    const result = section('Direct damage');
    const list = element('div', { className: 'repeatable-list' });
    (current.damages ?? []).forEach((damage, index) =>
      list.append(this.renderDamage(damage, index)),
    );
    if ((current.damages ?? []).length === 0) {
      list.append(
        element('p', { className: 'muted', text: 'No direct damage.' }),
      );
    }
    result.append(
      list,
      button('+ Add Direct Damage', () => {
        this.updateRecord(
          (record) => ({
            ...record,
            damages: [
              ...(record.damages ?? []),
              {
                damageType: 'DAMAGE_TYPE_HEAT',
                dmgDice: ['D6'],
                dmgBonus: 0,
                dmgStat: 'STAT_MND',
                dmgStatMult: 1,
              },
            ],
          }),
          true,
        );
      }),
    );
    return result;
  }

  private renderDamage(damage: AbilityDamage, index: number): HTMLDivElement {
    const item = element('div', { className: 'repeatable-item' });
    item.append(
      itemHeader(`Direct damage ${index + 1}`, () =>
        this.removeListItem('damages', index),
      ),
      this.renderDiceList(`damage-${index}`, damage.dmgDice, (dmgDice) =>
        this.updateListItem('damages', index, (value) => ({
          ...value,
          dmgDice,
        })),
      ),
    );
    const grid = element('div', { className: 'form-grid' });
    const type = selectInput(DAMAGE_TYPES, damage.damageType);
    type.addEventListener('change', () =>
      this.updateListItem('damages', index, (value) => ({
        ...value,
        damageType: type.value as DamageType,
      })),
    );
    const bonus = numberInput(damage.dmgBonus);
    bonus.addEventListener('input', () => {
      if (bonus.value !== '') {
        this.updateListItem('damages', index, (value) => ({
          ...value,
          dmgBonus: bonus.valueAsNumber,
        }));
      }
    });
    const stat = selectInput(STAT_TYPES, damage.dmgStat);
    stat.addEventListener('change', () =>
      this.updateListItem('damages', index, (value) => ({
        ...value,
        dmgStat: stat.value as StatType,
      })),
    );
    const mult = numberInput(damage.dmgStatMult);
    mult.step = 'any';
    mult.addEventListener('input', () => {
      if (mult.value !== '') {
        this.updateListItem('damages', index, (value) => ({
          ...value,
          dmgStatMult: mult.valueAsNumber,
        }));
      }
    });
    grid.append(
      field('Damage type', `damage-${index}-type`, type),
      field('Flat bonus', `damage-${index}-bonus`, bonus),
      field('Scaling stat', `damage-${index}-stat`, stat),
      field('Stat multiplier', `damage-${index}-multiplier`, mult),
    );
    item.append(grid);
    return item;
  }

  private renderDiceList(
    idPrefix: string,
    dice: readonly DieType[],
    onChange: (dice: DieType[]) => void,
  ): HTMLDivElement {
    const wrapper = element('div', { className: 'form-stack' });
    const list = element('div', { className: 'form-grid' });
    dice.forEach((die, index) => {
      const entry = element('div', { className: 'field' });
      const select = selectInput(DICE_TYPES, die);
      select.id = `${idPrefix}-die-${index}`;
      select.addEventListener('change', () => {
        const next = [...dice];
        next[index] = select.value as DieType;
        onChange(next);
      });
      entry.append(
        element('label', {
          text: `Die ${index + 1}`,
          attributes: { for: select.id },
        }),
        select,
      );
      if (dice.length > 1) {
        entry.append(
          button(
            'Remove die',
            () => {
              onChange(dice.filter((_, itemIndex) => itemIndex !== index));
              this.renderForm();
            },
            'button--danger button--small',
          ),
        );
      }
      list.append(entry);
    });
    wrapper.append(
      list,
      button(
        '+ Add Die',
        () => {
          onChange([...dice, 'D6']);
          this.renderForm();
        },
        'button--small',
      ),
    );
    return wrapper;
  }
}
