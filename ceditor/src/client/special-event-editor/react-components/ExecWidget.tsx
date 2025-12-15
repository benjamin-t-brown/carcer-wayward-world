import { useState } from 'react';
import { GameEvent } from '../../types/assets';
import { SearchInput } from '../../elements/SearchInput';
import { getAvailableFuncs } from '../eventRunner/EventRunner';

interface ExecWidgetProps {
  gameEvent: GameEvent | null;
}

const getCopyableFuncName = (funcName: string) => {
  return funcName.split('(')[0] + '()';
};

export function ExecWidget({ gameEvent }: ExecWidgetProps) {
  const [copiedVar, setCopiedVar] = useState<string | null>(null);
  const [hoverVar, setHoverVar] = useState<string | null>(null);
  const [copyTimeoutId, setCopyTimeoutId] = useState<NodeJS.Timeout | null>(
    null
  );

  const availableFuncs: string[] = getAvailableFuncs();

  const handleCopyVariable = async (funcName: string) => {
    if (copyTimeoutId) {
      clearTimeout(copyTimeoutId);
      setCopyTimeoutId(null);
    }
    try {
      await navigator.clipboard.writeText(funcName);
      setCopiedVar(funcName);
      setCopyTimeoutId(setTimeout(() => setCopiedVar(null), 2000));
    } catch (err) {
      console.error('Failed to copy variable name:', err);
    }
  };

  if (!gameEvent) {
    return null;
  }

  return (
    <div>
      <div>
        Search Functions{' '}
        <span
          style={{
            color: '#4ec9b0',
            fontSize: '11px',
            marginLeft: '10px',
            fontWeight: '500',
          }}
        >
          {copiedVar ? `✓ Copied! ${copiedVar}` : ''}
        </span>
      </div>
      <SearchInput
        items={availableFuncs}
        onSelect={(func) => {
          console.log('select', func);
          handleCopyVariable(getCopyableFuncName(func));
          setCopiedVar(getCopyableFuncName(func));
          setHoverVar(func);
        }}
        searchFields={(func: string) => [func]}
        renderItem={(func: string) => (
          <div style={{ fontFamily: 'courier new' }}>{func}</div>
        )}
        getItemKey={(func: string) => func}
      />
      <div
        onClick={() => {
          if (hoverVar) {
            handleCopyVariable(getCopyableFuncName(hoverVar));
            setCopiedVar(getCopyableFuncName(hoverVar));
          }
        }}
      >
        {hoverVar ? (
          <div
            style={{
              margin: '4px',
              display: 'inline-block',
              fontFamily: 'courier new',
            }}
          >
            {hoverVar}
          </div>
        ) : (
          <div style={{ margin: '0px', display: 'inline-block' }}>
            No function selected
          </div>
        )}
      </div>
    </div>
  );
}
