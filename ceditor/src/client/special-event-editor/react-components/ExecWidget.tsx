import { useState } from 'react';
import { GameEvent, Variable } from '../../types/assets';
import { useAssets } from '../../contexts/AssetsContext';
import { AccessibleVariable, getVarsFromNode } from '../nodeHelpers';
import { SearchInput } from '../../elements/SearchInput';
import { EventRunner, getAvailableFuncs } from '../eventRunner/EventRunner';

interface ExecWidgetProps {
  gameEvent: GameEvent | null;
}

export function ExecWidget({ gameEvent }: ExecWidgetProps) {
  const { gameEvents } = useAssets();
  const [copiedVar, setCopiedVar] = useState<string | null>(null);
  const [hoverVar, setHoverVar] = useState<string | null>(null);
  // const [includeImports, setIncludeImports] = useState<boolean>(false);
  // const [isExpanded, setIsExpanded] = useState<boolean>(false);

  const availableFuncs: string[] = getAvailableFuncs();

  // const accessibleVars = gameEvent
  //   ? getVarsFromNode(gameEvent, gameEvents)
  //   : [];

  // const handleCopyVariable = async (variableName: string) => {
  //   try {
  //     await navigator.clipboard.writeText('@' + variableName);
  //     setCopiedVar(variableName);
  //     setTimeout(() => setCopiedVar(null), 2000);
  //   } catch (err) {
  //     console.error('Failed to copy variable name:', err);
  //   }
  // };

  if (!gameEvent) {
    return null;
  }

  return (
    <div>
      <div>Search Functions</div>
      <SearchInput
        items={availableFuncs}
        onSelect={(func) => {
          console.log('select', func);
          setCopiedVar(func);
          setHoverVar(func);
        }}
        searchFields={(func: string) => [func]}
        renderItem={(func: string) => <div style={{ fontFamily: 'courier new' }}>{func}</div>}
        getItemKey={(func: string) => func}
      />
      <div>
        {hoverVar ? (
          <div style={{ margin: '4px', display: 'inline-block', fontFamily: 'courier new' }}>
            {hoverVar}
          </div>
        ) : (
          <div style={{ margin: '4px', display: 'inline-block' }}>
            No function selected
          </div>
        )}
      </div>
    </div>
  );
}
