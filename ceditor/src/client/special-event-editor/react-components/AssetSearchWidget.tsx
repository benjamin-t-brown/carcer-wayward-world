import { useMemo, useState } from 'react';
import { GameEvent } from '../../types/assets';
import { useAssets } from '../../contexts/AssetsContext';
import { getVarsFromNode } from '../nodeHelpers';
import { SearchInput } from '../../elements/SearchInput';
import { getAvailableFuncs } from '../eventRunner/EventRunner';

export type AssetSearchKind =
  | 'variable'
  | 'function'
  | 'item'
  | 'character'
  | 'quest'
  | 'questStep'
  | 'questSubStep';

const SEARCH_KINDS: { id: AssetSearchKind; label: string; color: string }[] = [
  { id: 'variable', label: 'Variables', color: '#4ec9b0' },
  { id: 'function', label: 'Functions', color: '#c586c0' },
  { id: 'item', label: 'Items', color: '#dcdcaa' },
  { id: 'character', label: 'Characters', color: '#b5cea8' },
  { id: 'quest', label: 'Quests', color: '#569cd6' },
  { id: 'questStep', label: 'Steps', color: '#9cdcfe' },
  { id: 'questSubStep', label: 'Sub-steps', color: '#ce9178' },
];

const KIND_STORAGE_KEY = 'ceditor.seAssetSearch.kinds';
const CHARACTER_KIND_MIGRATION_KEY = 'ceditor.seAssetSearch.addedCharacterKind';

interface SearchHit {
  kind: AssetSearchKind;
  key: string;
  copyText: string;
  title: string;
  detail: string;
  searchFields: string[];
}

function loadEnabledKinds(): Set<AssetSearchKind> {
  const allKinds = SEARCH_KINDS.map((kind) => kind.id);
  try {
    const raw = localStorage.getItem(KIND_STORAGE_KEY);
    if (!raw) {
      return new Set(allKinds);
    }
    const parsed = JSON.parse(raw) as unknown;
    if (!Array.isArray(parsed)) {
      return new Set(allKinds);
    }
    const valid = parsed.filter((id): id is AssetSearchKind =>
      SEARCH_KINDS.some((kind) => kind.id === id),
    );
    const enabled = new Set(valid.length > 0 ? valid : allKinds);
    if (!localStorage.getItem(CHARACTER_KIND_MIGRATION_KEY)) {
      enabled.add('character');
      localStorage.setItem(CHARACTER_KIND_MIGRATION_KEY, '1');
      saveEnabledKinds(enabled);
    }
    return enabled;
  } catch {
    return new Set(allKinds);
  }
}

function saveEnabledKinds(kinds: Set<AssetSearchKind>) {
  localStorage.setItem(KIND_STORAGE_KEY, JSON.stringify([...kinds]));
}

function getCopyableFuncName(funcName: string) {
  return funcName.split('(')[0] + '()';
}

function kindMeta(kind: AssetSearchKind) {
  return SEARCH_KINDS.find((entry) => entry.id === kind) ?? SEARCH_KINDS[0];
}

interface AssetSearchWidgetProps {
  gameEvent: GameEvent | null;
}

export function AssetSearchWidget({ gameEvent }: AssetSearchWidgetProps) {
  const { gameEvents, items, characters, quests } = useAssets();
  const [enabledKinds, setEnabledKinds] = useState<Set<AssetSearchKind>>(
    loadEnabledKinds,
  );
  const [copiedText, setCopiedText] = useState<string | null>(null);
  const [selected, setSelected] = useState<SearchHit | null>(null);
  const [copyTimeoutId, setCopyTimeoutId] = useState<number | null>(null);

  const availableFuncs = useMemo(() => getAvailableFuncs(), []);
  const accessibleVars = useMemo(
    () => (gameEvent ? getVarsFromNode(gameEvent, gameEvents) : []),
    [gameEvent, gameEvents],
  );

  const hits = useMemo(() => {
    const next: SearchHit[] = [];
    if (enabledKinds.has('variable')) {
      for (const variable of accessibleVars) {
        next.push({
          kind: 'variable',
          key: `variable:${variable.source}:${variable.key}`,
          copyText: `@${variable.key}`,
          title: `@${variable.key}`,
          detail:
            variable.source === gameEvent?.id
              ? `= ${variable.value}`
              : `= ${variable.value} (${variable.source})`,
          searchFields: [variable.key, variable.value, variable.source],
        });
      }
    }
    if (enabledKinds.has('function')) {
      for (const func of availableFuncs) {
        next.push({
          kind: 'function',
          key: `function:${func}`,
          copyText: getCopyableFuncName(func),
          title: func,
          detail: '',
          searchFields: [func, getCopyableFuncName(func)],
        });
      }
    }
    if (enabledKinds.has('item')) {
      for (const item of items) {
        next.push({
          kind: 'item',
          key: `item:${item.name}`,
          copyText: item.name,
          title: item.name,
          detail: item.label && item.label !== item.name ? item.label : '',
          searchFields: [item.name, item.label ?? ''],
        });
      }
    }
    if (enabledKinds.has('character')) {
      for (const character of characters) {
        const talkName = character.talk?.talkName?.trim() ?? '';
        const portraitName = character.talk?.portraitName?.trim() ?? '';
        const labelPart =
          character.label && character.label !== character.name
            ? character.label
            : '';
        const detail = [labelPart, talkName].filter(Boolean).join(' — ');
        next.push({
          kind: 'character',
          key: `character:${character.name}`,
          copyText: character.name,
          title: character.name,
          detail,
          searchFields: [
            character.name,
            character.label ?? '',
            character.type ?? '',
            talkName,
            portraitName,
          ],
        });
      }
    }
    if (
      enabledKinds.has('quest') ||
      enabledKinds.has('questStep') ||
      enabledKinds.has('questSubStep')
    ) {
      for (const quest of quests) {
        const questLabel = quest.label || quest.id;
        if (enabledKinds.has('quest')) {
          const stepIds = (quest.steps ?? [])
            .map((step) => step.id)
            .filter((id) => id.trim().length > 0);
          const stepList = stepIds.join(', ');
          const labelPart =
            questLabel !== quest.id ? questLabel : '';
          const detail = [labelPart, stepList].filter(Boolean).join(' — ');
          next.push({
            kind: 'quest',
            key: `quest:${quest.id}`,
            copyText: quest.id,
            title: quest.id,
            detail,
            searchFields: [
              quest.id,
              quest.label ?? '',
              quest.description ?? '',
              ...stepIds,
            ],
          });
        }
        for (const step of quest.steps ?? []) {
          if (enabledKinds.has('questStep')) {
            next.push({
              kind: 'questStep',
              key: `questStep:${quest.id}:${step.id}`,
              copyText: step.id,
              title: step.id,
              detail: `${questLabel}${step.label ? ` / ${step.label}` : ''}`,
              searchFields: [
                step.id,
                step.label ?? '',
                step.description ?? '',
                quest.id,
                quest.label ?? '',
              ],
            });
          }
          if (enabledKinds.has('questSubStep')) {
            for (const subStep of step.subSteps ?? []) {
              next.push({
                kind: 'questSubStep',
                key: `questSubStep:${quest.id}:${step.id}:${subStep.id}`,
                copyText: subStep.id,
                title: subStep.id,
                detail: `${questLabel} / ${step.label || step.id}${
                  subStep.label ? ` / ${subStep.label}` : ''
                }`,
                searchFields: [
                  subStep.id,
                  subStep.label ?? '',
                  subStep.description ?? '',
                  step.id,
                  step.label ?? '',
                  quest.id,
                  quest.label ?? '',
                ],
              });
            }
          }
        }
      }
    }
    return next;
  }, [
    accessibleVars,
    availableFuncs,
    characters,
    enabledKinds,
    gameEvent?.id,
    items,
    quests,
  ]);

  const copyHit = async (hit: SearchHit) => {
    if (copyTimeoutId) {
      window.clearTimeout(copyTimeoutId);
    }
    try {
      await navigator.clipboard.writeText(hit.copyText);
      setCopiedText(hit.copyText);
      setCopyTimeoutId(window.setTimeout(() => setCopiedText(null), 2000));
    } catch (err) {
      console.error('Failed to copy search result:', err);
    }
  };

  const toggleKind = (kind: AssetSearchKind) => {
    setEnabledKinds((current) => {
      const next = new Set(current);
      if (next.has(kind)) {
        next.delete(kind);
      } else {
        next.add(kind);
      }
      saveEnabledKinds(next);
      return next;
    });
  };

  return (
    <div className="asset-search-widget" style={{ width: '100%', marginBottom: '20px' }}>
      <div
        style={{
          display: 'flex',
          flexWrap: 'wrap',
          gap: '10px 14px',
          marginBottom: '8px',
          alignItems: 'center',
        }}
      >
        {SEARCH_KINDS.map((kind) => (
          <label
            key={kind.id}
            style={{
              display: 'inline-flex',
              alignItems: 'center',
              gap: '5px',
              fontSize: '12px',
              color: '#d4d4d4',
              cursor: 'pointer',
              userSelect: 'none',
            }}
          >
            <input
              type="checkbox"
              checked={enabledKinds.has(kind.id)}
              onChange={() => toggleKind(kind.id)}
            />
            <span style={{ color: kind.color }}>{kind.label}</span>
          </label>
        ))}
        <span
          style={{
            color: '#4ec9b0',
            fontSize: '11px',
            fontWeight: 500,
            marginLeft: 'auto',
          }}
        >
          {copiedText ? `Copied ${copiedText}` : ''}
        </span>
      </div>
      <SearchInput
        items={hits}
        placeholder="Search variables, functions, items, characters, quests..."
        requireSearchTerm
        onSelect={(hit) => {
          setSelected(hit);
          void copyHit(hit);
        }}
        searchFields={(hit) => hit.searchFields}
        getItemKey={(hit) => hit.key}
        renderItem={(hit) => <HitDisplay hit={hit} />}
      />
      <div
        style={{ marginTop: '6px', cursor: selected ? 'pointer' : 'default' }}
        onClick={() => {
          if (selected) {
            void copyHit(selected);
          }
        }}
      >
        {selected ? (
          <HitDisplay hit={selected} wrap />
        ) : (
          <span style={{ fontSize: '12px', color: '#858585' }}>
            Type to search, then click a result to copy it
          </span>
        )}
      </div>
    </div>
  );
}

function HitDisplay({ hit, wrap = false }: { hit: SearchHit; wrap?: boolean }) {
  const meta = kindMeta(hit.kind);
  return (
    <div
      style={{
        display: 'flex',
        alignItems: wrap ? 'flex-start' : 'baseline',
        gap: '8px',
        fontSize: '12px',
        minWidth: 0,
        flexWrap: wrap ? 'wrap' : 'nowrap',
      }}
    >
      <span
        style={{
          color: meta.color,
          backgroundColor: '#1e1e1e',
          padding: '1px 6px',
          borderRadius: '3px',
          fontSize: '10px',
          textTransform: 'uppercase',
          flexShrink: 0,
        }}
      >
        {meta.label}
      </span>
      <span
        style={{
          fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Consolas, monospace',
          color: '#d4d4d4',
          overflow: wrap ? 'visible' : 'hidden',
          textOverflow: wrap ? undefined : 'ellipsis',
          whiteSpace: wrap ? 'normal' : 'nowrap',
        }}
      >
        {hit.title}
      </span>
      {hit.detail ? (
        <span
          style={{
            color: '#858585',
            overflow: wrap ? 'visible' : 'hidden',
            textOverflow: wrap ? undefined : 'ellipsis',
            whiteSpace: wrap ? 'normal' : 'nowrap',
          }}
        >
          {hit.detail}
        </span>
      ) : null}
    </div>
  );
}
