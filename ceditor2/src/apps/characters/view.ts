import type { JsonArray, JsonObject } from '../../core/database/index.js';
import {
  CHARACTER_BEHAVIORS,
  CHARACTER_TYPES,
  COMBAT_BEHAVIORS,
  cloneCharacterRecord,
  createDefaultCharacterRecord,
  createDefaultCharacterStats,
  createUniqueCharacterName,
  parseCharacterCollection,
  type CharacterBehaviorName,
  type CharacterRecord,
  type CharacterStats,
  type CharacterType,
  type CombatBehaviorName,
} from '../../core/domain/characters/index.js';
import { element } from '../../core/ui/dom.js';
import {
  characterSpriteName,
  createEntitySpritePreview,
  createMediaPickerField,
  type DatabasePageContext,
} from '../../core/ui/index.js';
import {
  findDeepLinkedCharacter,
  matchesCharacterSearch,
  withCharacterSelection,
} from './editorModel.js';

type CharacterObjectKey =
  | 'stats'
  | 'talk'
  | 'behavior'
  | 'combat'
  | 'combatBehavior'
  | 'sound'
  | 'vision';
type StatPath =
  | readonly ['generic', string]
  | readonly ['skills', string]
  | readonly ['trainable', 'weapon' | 'magic' | 'body', string];
interface StatGroup {
  readonly title: string;
  readonly fields: readonly { label: string; path: StatPath }[];
}

function twoLevel(
  title: string,
  group: 'generic' | 'skills',
  fields: readonly (readonly [string, string])[],
): StatGroup {
  return {
    title,
    fields: fields.map(([label, key]) => ({ label, path: [group, key] })),
  };
}
function threeLevel(
  title: string,
  group: 'weapon' | 'magic' | 'body',
  fields: readonly (readonly [string, string])[],
): StatGroup {
  return {
    title,
    fields: fields.map(([label, key]) => ({
      label,
      path: ['trainable', group, key],
    })),
  };
}

const STAT_GROUPS: readonly StatGroup[] = [
  twoLevel('Generic', 'generic', [
    ['Strength', 'str'],
    ['Mind', 'mnd'],
    ['Constitution', 'con'],
    ['Agility', 'agi'],
    ['Luck', 'lck'],
  ]),
  threeLevel('Weapon mastery', 'weapon', [
    ['Edged', 'edged'],
    ['Pole', 'pole'],
    ['Blunt', 'blunt'],
    ['Range', 'range'],
    ['Unarmed', 'unarmed'],
  ]),
  threeLevel('Magic mastery', 'magic', [
    ['Mana', 'mana'],
    ['Ability power', 'abilityPower'],
    ['Attunement', 'attunement'],
    ['Faith', 'faith'],
    ['Lore', 'lore'],
  ]),
  threeLevel('Body mastery', 'body', [
    ['Physical resistance', 'resistPhysical'],
    ['Magical resistance', 'resistMagical'],
    ['Healing effectiveness', 'healingEffectiveness'],
    ['Damage reduction', 'dr'],
    ['Armor training', 'armorTraining'],
  ]),
  twoLevel('Skills', 'skills', [
    ['Trickery', 'trickery'],
    ['Stealth', 'stealth'],
    ['Social', 'social'],
    ['Magic item use', 'magicItemUse'],
    ['Cooking', 'cooking'],
    ['Acrobatics', 'acrobatics'],
    ['Survival', 'survival'],
    ['Focus', 'focus'],
    ['Conditioning', 'conditioning'],
  ]),
];

function recordString(value: unknown, key: string): string | null {
  if (typeof value !== 'object' || value === null || Array.isArray(value))
    return null;
  const fieldValue = (value as Record<string, unknown>)[key];
  return typeof fieldValue === 'string' && fieldValue.trim()
    ? fieldValue
    : null;
}

function field(
  labelText: string,
  id: string,
  control: HTMLInputElement | HTMLSelectElement,
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

function textInput(value = ''): HTMLInputElement {
  const input = element('input');
  input.type = 'text';
  input.value = value;
  input.autocomplete = 'off';
  return input;
}
function numberInput(value?: number): HTMLInputElement {
  const input = element('input');
  input.type = 'number';
  input.step = '1';
  input.value = value === undefined ? '' : String(value);
  return input;
}
function selectInput(
  values: readonly string[],
  current: string,
  includeBlank = false,
): HTMLSelectElement {
  const select = element('select');
  const options = includeBlank ? ['', ...values] : [...values];
  if (current && !options.includes(current)) options.unshift(current);
  options.forEach((value) => {
    const option = element('option', { text: value || '(none)' });
    option.value = value;
    select.append(option);
  });
  select.value = current;
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
  if (description)
    result.append(
      element('p', {
        className: 'form-section__description',
        text: description,
      }),
    );
  return result;
}
function omitKey(object: JsonObject, key: string): JsonObject {
  const next = { ...object };
  delete next[key];
  return next;
}
function statValue(
  stats: CharacterStats | undefined,
  path: StatPath,
): number | undefined {
  if (path[0] === 'generic' || path[0] === 'skills') {
    return stats?.[path[0]]?.[path[1]] as number | undefined;
  }
  return stats?.trainable?.[path[1]]?.[path[2]] as number | undefined;
}
function withStat(
  record: CharacterRecord,
  path: StatPath,
  value: number | undefined,
): CharacterRecord {
  const stats = { ...(record.stats ?? {}) } as CharacterStats;
  if (path[0] === 'generic' || path[0] === 'skills') {
    const group = { ...(stats[path[0]] ?? {}) } as JsonObject;
    if (value === undefined) delete group[path[1]];
    else group[path[1]] = value;
    stats[path[0]] = group;
  } else {
    const trainable = { ...(stats.trainable ?? {}) };
    const group = { ...(trainable[path[1]] ?? {}) } as JsonObject;
    if (value === undefined) delete group[path[2]];
    else group[path[2]] = value;
    trainable[path[1]] = group;
    stats.trainable = trainable;
  }
  return { ...record, stats };
}

export class CharactersEditor {
  private records: CharacterRecord[];
  private selectedIndex: number;
  private searchTerm = '';
  private readonly talkEvents: readonly { id: string; label: string }[];
  private readonly statusNames: readonly string[];
  private readonly list = element('ul', { className: 'entity-list' });
  private readonly resultCount = element('p', { className: 'muted' });
  private readonly main = element('section', { className: 'editor-main' });

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.records = parseCharacterCollection(
      context.session.collection('characters'),
    );
    this.selectedIndex = findDeepLinkedCharacter(this.records, initialUrl);
    this.talkEvents = context.session
      .collection('specialEvents')
      .filter((record) => recordString(record, 'eventType') === 'TALK')
      .map((record) => {
        const id = recordString(record, 'id') ?? '';
        return { id, label: recordString(record, 'title') ?? id };
      })
      .filter(({ id }) => Boolean(id))
      .sort((left, right) => left.label.localeCompare(right.label));
    this.statusNames = context.session
      .collection('statusEffects')
      .map((record) => recordString(record, 'name'))
      .filter((name): name is string => name !== null)
      .sort((left, right) => left.localeCompare(right));
  }

  mount(): void {
    const layout = element('div', { className: 'editor-layout' });
    const sidebar = element('aside', {
      className: 'editor-sidebar',
      attributes: { 'aria-label': 'Characters' },
    });
    const controls = element('div', { className: 'editor-sidebar__controls' });
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Name, label, or type';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    controls.append(
      field('Search characters', 'character-search', search),
      button('+ New Character', () => this.createRecord(), 'button--primary'),
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
  private selected(): CharacterRecord | undefined {
    return this.records[this.selectedIndex];
  }
  private commit(next?: CharacterRecord): void {
    if (next && this.selectedIndex >= 0)
      this.records[this.selectedIndex] = next;
    this.context.session.replaceCollection(
      'characters',
      this.records as JsonArray,
    );
    this.context.notifyChanged();
  }
  private updateRecord(
    update: (record: CharacterRecord) => CharacterRecord,
    renderForm = false,
  ): void {
    const current = this.selected();
    if (!current) return;
    const next = update(current);
    this.commit(next);
    window.history.replaceState(
      null,
      '',
      withCharacterSelection(new URL(window.location.href), next.name),
    );
    this.renderList();
    if (renderForm) this.renderForm();
  }
  private updateObject(
    key: Exclude<CharacterObjectKey, 'stats'>,
    update: (object: JsonObject) => JsonObject,
  ): void {
    this.updateRecord((record) => ({
      ...record,
      [key]: update((record[key] ?? {}) as JsonObject),
    }));
  }
  private removeGroup(key: CharacterObjectKey): void {
    this.updateRecord(
      (record) => omitKey(record, key) as CharacterRecord,
      true,
    );
  }
  private setSelection(index: number): void {
    this.selectedIndex = index;
    window.history.replaceState(
      null,
      '',
      withCharacterSelection(
        new URL(window.location.href),
        this.selected()?.name,
      ),
    );
    this.renderList();
    this.renderForm();
  }
  private createRecord(): void {
    const names = this.records.map(({ name }) => name);
    const name = names.includes('NEW_CHARACTER')
      ? createUniqueCharacterName('NEW_CHARACTER', names)
      : 'NEW_CHARACTER';
    this.records.push(createDefaultCharacterRecord(name));
    this.searchTerm = '';
    const search =
      this.root.querySelector<HTMLInputElement>('#character-search');
    if (search) search.value = '';
    this.commit();
    this.setSelection(this.records.length - 1);
  }
  private cloneSelected(): void {
    const current = this.selected();
    if (!current) return;
    const clone = cloneCharacterRecord(
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
        `Delete character "${current.name}"? Map references are not changed automatically.`,
      )
    )
      return;
    this.records.splice(this.selectedIndex, 1);
    this.commit();
    this.setSelection(-1);
  }

  private renderList(): void {
    this.list.replaceChildren();
    const matching = this.records
      .map((record, index) => ({ record, index }))
      .filter(({ record }) => matchesCharacterSearch(record, this.searchTerm));
    this.resultCount.textContent = `${matching.length} of ${this.records.length}`;
    if (!matching.length) {
      this.list.append(
        element('li', {
          className: 'empty-state',
          text: 'No characters found.',
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
          text: record.label || record.name || '(unnamed character)',
        }),
        element('span', {
          className: 'entity-card__subtitle',
          text: `(${record.name || 'No ID'})`,
        }),
      );
      select.append(
        createEntitySpritePreview(
          characterSpriteName(record.spritesheet, record.spriteOffset),
        ),
        body,
      );
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
          text: 'Select a character to edit, or create a new one.',
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
      this.renderStats(current),
      this.renderTalk(current),
      this.renderBehavior(current),
      this.renderCombat(current),
      this.renderCombatBehavior(current),
      this.renderSound(current),
      this.renderStatuses(current),
      this.renderVision(current),
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

  private renderIdentity(current: CharacterRecord): HTMLFieldSetElement {
    const result = section('Character and sprite');
    const grid = element('div', { className: 'form-grid' });
    const type = selectInput(CHARACTER_TYPES, current.type);
    type.addEventListener('change', () =>
      this.updateRecord((record) => ({
        ...record,
        type: type.value as CharacterType,
      })),
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
    const spritesheet = textInput(current.spritesheet);
    spritesheet.required = true;
    spritesheet.addEventListener('input', () =>
      this.updateRecord((record) => ({
        ...record,
        spritesheet: spritesheet.value,
      })),
    );
    const offset = textInput(String(current.spriteOffset));
    offset.required = true;
    offset.inputMode = 'numeric';
    offset.addEventListener('input', () => {
      const value =
        typeof current.spriteOffset === 'number' && /^-?\d+$/.test(offset.value)
          ? Number(offset.value)
          : offset.value;
      this.updateRecord((record) => ({ ...record, spriteOffset: value }));
    });
    grid.append(
      field('Type', 'character-type', type),
      field('Name (ID)', 'character-name', name),
      field('Label', 'character-label', label),
      createMediaPickerField({
        id: 'character-sprite-picker',
        label: 'Character sprite',
        kind: 'sprite',
        value: characterSpriteName(current.spritesheet, current.spriteOffset),
        readOnly: true,
        help: 'Choosing a sprite updates the spritesheet and offset fields together.',
        onChange: (_value, choice) => {
          if (!choice || !('pictureAlias' in choice)) return;
          this.updateRecord(
            (record) => ({
              ...record,
              spritesheet: choice.pictureAlias,
              spriteOffset: choice.index,
            }),
            true,
          );
        },
      }),
      field('Spritesheet', 'character-spritesheet', spritesheet),
      field(
        'Sprite offset',
        'character-sprite-offset',
        offset,
        'Integer index or named string offset.',
      ),
    );
    result.append(grid);
    return result;
  }

  private renderStats(current: CharacterRecord): HTMLFieldSetElement {
    const result = section(
      'Stats',
      current.stats
        ? 'Blank fields are omitted and use the game default of zero.'
        : 'This optional group is absent. Enter a value or add complete zero defaults.',
    );
    STAT_GROUPS.forEach((group) => {
      result.append(
        element('h3', { className: 'form-section__title', text: group.title }),
      );
      const grid = element('div', { className: 'form-grid' });
      group.fields.forEach(({ label, path }) => {
        const input = numberInput(statValue(current.stats, path));
        input.addEventListener('input', () =>
          this.updateRecord((record) =>
            withStat(
              record,
              path,
              input.value === '' ? undefined : input.valueAsNumber,
            ),
          ),
        );
        grid.append(field(label, `character-stat-${path.join('-')}`, input));
      });
      result.append(grid);
    });
    result.append(
      current.stats
        ? button(
            'Remove stats',
            () => this.removeGroup('stats'),
            'button--danger',
          )
        : button('Add zero defaults', () =>
            this.updateRecord(
              (record) => ({
                ...record,
                stats: createDefaultCharacterStats(),
              }),
              true,
            ),
          ),
    );
    return result;
  }

  private renderTalk(current: CharacterRecord): HTMLFieldSetElement {
    const result = section(
      'Talk',
      'References TALK events from the complete database.',
    );
    const grid = element('div', { className: 'form-grid' });
    const talkName = textInput(current.talk?.talkName);
    talkName.setAttribute('list', 'character-talk-events');
    talkName.addEventListener('input', () =>
      this.updateObject('talk', (talk) => ({
        ...talk,
        talkName: talkName.value,
      })),
    );
    const datalist = element('datalist', {
      attributes: { id: 'character-talk-events' },
    });
    this.talkEvents.forEach(({ id, label }) =>
      datalist.append(element('option', { attributes: { value: id, label } })),
    );
    grid.append(
      field('Talk event', 'character-talk-name', talkName),
      createMediaPickerField({
        id: 'character-portrait-name',
        label: 'Portrait sprite',
        kind: 'sprite',
        value: current.talk?.portraitName ?? '',
        onChange: (value) =>
          this.updateObject('talk', (talk) => ({
            ...talk,
            portraitName: value,
          })),
      }),
    );
    result.append(grid, datalist);
    if (current.talk?.talkName)
      result.append(
        element('a', {
          text: 'Open talk event',
          attributes: {
            href: `../events/?event=${encodeURIComponent(current.talk.talkName)}`,
            target: '_blank',
            rel: 'noopener noreferrer',
          },
        }),
      );
    if (current.talk)
      result.append(
        button('Remove talk', () => this.removeGroup('talk'), 'button--danger'),
      );
    return result;
  }

  private renderBehavior(current: CharacterRecord): HTMLFieldSetElement {
    const result = section('Map behavior');
    const behavior = selectInput(
      CHARACTER_BEHAVIORS,
      current.behavior?.behaviorName ?? '',
      true,
    );
    behavior.addEventListener('change', () =>
      this.updateObject('behavior', (value) =>
        behavior.value
          ? { ...value, behaviorName: behavior.value as CharacterBehaviorName }
          : omitKey(value, 'behaviorName'),
      ),
    );
    result.append(field('Behavior', 'character-behavior', behavior));
    if (current.behavior)
      result.append(
        button(
          'Remove behavior',
          () => this.removeGroup('behavior'),
          'button--danger',
        ),
      );
    return result;
  }

  private renderCombat(current: CharacterRecord): HTMLFieldSetElement {
    const result = section('Combat');
    const grid = element('div', { className: 'form-grid' });
    const addNumber = (label: string, key: 'hp' | 'mp') => {
      const input = numberInput(current.combat?.[key]);
      input.addEventListener('input', () =>
        this.updateObject('combat', (combat) =>
          input.value === ''
            ? omitKey(combat, key)
            : { ...combat, [key]: input.valueAsNumber },
        ),
      );
      grid.append(field(label, `character-combat-${key}`, input));
    };
    addNumber('HP', 'hp');
    addNumber('MP', 'mp');
    const dropTable = textInput(current.combat?.dropTable);
    dropTable.addEventListener('input', () =>
      this.updateObject('combat', (combat) => ({
        ...combat,
        dropTable: dropTable.value,
      })),
    );
    grid.append(field('Drop table', 'character-combat-drop-table', dropTable));
    result.append(grid);
    if (current.combat?.stats)
      result.append(
        element('p', {
          className: 'status status--info',
          text: 'Legacy combat.stats is retained unchanged and overrides matching generic stats in the game loader.',
        }),
      );
    if (current.combat)
      result.append(
        button(
          'Remove combat',
          () => this.removeGroup('combat'),
          'button--danger',
        ),
      );
    return result;
  }

  private renderCombatBehavior(current: CharacterRecord): HTMLFieldSetElement {
    const result = section('Combat behavior');
    const grid = element('div', { className: 'form-grid' });
    (['town', 'combat'] as const).forEach((key) => {
      const select = selectInput(
        COMBAT_BEHAVIORS,
        current.combatBehavior?.[key] ?? '',
        true,
      );
      select.addEventListener('change', () =>
        this.updateObject('combatBehavior', (behavior) =>
          select.value
            ? { ...behavior, [key]: select.value as CombatBehaviorName }
            : omitKey(behavior, key),
        ),
      );
      grid.append(
        field(
          key === 'town' ? 'Town' : 'Combat',
          `character-combat-behavior-${key}`,
          select,
        ),
      );
    });
    result.append(grid);
    if (current.combatBehavior)
      result.append(
        button(
          'Remove combat behavior',
          () => this.removeGroup('combatBehavior'),
          'button--danger',
        ),
      );
    return result;
  }

  private renderSound(current: CharacterRecord): HTMLFieldSetElement {
    const result = section(
      'Sound',
      'Editing writes current *SoundName fields; legacy aliases remain intact.',
    );
    const grid = element('div', { className: 'form-grid' });
    grid.append(
      createMediaPickerField({
        id: 'character-death-sound',
        label: 'Death sound',
        kind: 'sound',
        value: current.sound?.deathSoundName ?? current.sound?.deathSound ?? '',
        onChange: (value) =>
          this.updateObject('sound', (sound) => ({
            ...sound,
            deathSoundName: value,
          })),
      }),
      createMediaPickerField({
        id: 'character-weapon-sound',
        label: 'Weapon sound',
        kind: 'sound',
        value:
          current.sound?.weaponSoundName ?? current.sound?.weaponSound ?? '',
        onChange: (value) =>
          this.updateObject('sound', (sound) => ({
            ...sound,
            weaponSoundName: value,
          })),
      }),
    );
    result.append(grid);
    if (current.sound)
      result.append(
        button(
          'Remove sound',
          () => this.removeGroup('sound'),
          'button--danger',
        ),
      );
    return result;
  }

  private renderStatuses(current: CharacterRecord): HTMLFieldSetElement {
    const result = section(
      'Starting statuses',
      'References status effects from the complete database.',
    );
    const list = element('div', { className: 'repeatable-list' });
    (current.statuses ?? []).forEach((status, index) => {
      const item = element('div', { className: 'repeatable-item' });
      const header = element('div', { className: 'repeatable-item__header' });
      header.append(
        element('h3', {
          className: 'repeatable-item__title',
          text: `Status ${index + 1}`,
        }),
        button(
          'Remove',
          () =>
            this.updateRecord(
              (record) => ({
                ...record,
                statuses: (record.statuses ?? []).filter(
                  (_, itemIndex) => itemIndex !== index,
                ),
              }),
              true,
            ),
          'button--danger button--small',
        ),
      );
      const input = textInput(status.status);
      input.setAttribute('list', 'character-status-options');
      input.addEventListener('input', () =>
        this.updateRecord((record) => ({
          ...record,
          statuses: (record.statuses ?? []).map((value, itemIndex) =>
            itemIndex === index ? { ...value, status: input.value } : value,
          ),
        })),
      );
      item.append(
        header,
        field('Status effect', `character-status-${index}`, input),
      );
      list.append(item);
    });
    const datalist = element('datalist', {
      attributes: { id: 'character-status-options' },
    });
    this.statusNames.forEach((name) =>
      datalist.append(element('option', { attributes: { value: name } })),
    );
    result.append(
      list,
      datalist,
      button(
        '+ Add Status',
        () =>
          this.updateRecord(
            (record) => ({
              ...record,
              statuses: [...(record.statuses ?? []), { status: '' }],
            }),
            true,
          ),
        'button--primary',
      ),
    );
    return result;
  }

  private renderVision(current: CharacterRecord): HTMLFieldSetElement {
    const result = section('Vision');
    const radius = numberInput(current.vision?.radius);
    radius.min = '0';
    radius.addEventListener('input', () =>
      this.updateObject('vision', (vision) =>
        radius.value === ''
          ? omitKey(vision, 'radius')
          : { ...vision, radius: radius.valueAsNumber },
      ),
    );
    result.append(field('Radius', 'character-vision-radius', radius));
    if (current.vision)
      result.append(
        button(
          'Remove vision',
          () => this.removeGroup('vision'),
          'button--danger',
        ),
      );
    return result;
  }
}
