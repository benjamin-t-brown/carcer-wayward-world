import { useState } from 'react';
import { GameEvent } from '../../types/assets';
import { useAssets } from '../../contexts/AssetsContext';
import { AccessibleVariable, getVarsFromNode } from '../nodeHelpers';
import { SearchInput } from '../../elements/SearchInput';

interface VariableWidgetProps {
  gameEvent: GameEvent | null;
}

const VarDisplay = ({
  variable,
  gameEvent,
}: {
  variable: AccessibleVariable;
  gameEvent: GameEvent;
}) => {
  return (
    <div
      style={{
        margin: '4px',
        display: 'inline-block',
      }}
    >
      {variable.key ? (
        <>
          <span
            style={{
              color: variable.source === gameEvent.id ? '#4ec9b0' : 'orange',
              backgroundColor: '#252526',
              padding: '4px',
            }}
          >
            @{variable.key}
          </span>{' '}
          ={' '}
          <span
            style={{
              fontFamily: 'courier new',
            }}
          >
            {variable.value}
          </span>
          {variable.source !== gameEvent.id ? ` (${variable.source})` : ''}
        </>
      ) : (
        <span style={{ padding: '0px' }}>No variable selected</span>
      )}
    </div>
  );
};

export function VariableWidget({ gameEvent }: VariableWidgetProps) {
  const { gameEvents } = useAssets();
  const [copiedVar, setCopiedVar] = useState<string | null>(null);
  const [hoverVar, setHoverVar] = useState<AccessibleVariable | null>(null);
  const [copyTimeoutId, setCopyTimeoutId] = useState<NodeJS.Timeout | null>(
    null
  );

  const accessibleVars = gameEvent
    ? getVarsFromNode(gameEvent, gameEvents)
    : [];

  const handleCopyVariable = async (variableName: string) => {
    if (copyTimeoutId) {
      clearTimeout(copyTimeoutId);
      setCopyTimeoutId(null);
    }
    try {
      await navigator.clipboard.writeText('@' + variableName);
      setCopiedVar(variableName);
      setTimeout(() => setCopiedVar(null), 2000);
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
        Search Variables{' '}
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
        items={accessibleVars}
        onSelect={(variable) => {
          console.log('select', variable);
          handleCopyVariable(variable.key);
          setCopiedVar('@' + variable.key);
          setHoverVar(variable);
        }}
        searchFields={(variable: AccessibleVariable) => [variable.key]}
        renderItem={(variable) => (
          <VarDisplay variable={variable} gameEvent={gameEvent} />
        )}
        getItemKey={(variable) => variable.key}
      />
      <div
        onClick={() => {
          if (hoverVar?.key) {
            handleCopyVariable(hoverVar?.key ?? '');
            setCopiedVar('@' + (hoverVar?.key ?? ''));
          }
        }}
      >
        {hoverVar ? (
          <VarDisplay variable={hoverVar} gameEvent={gameEvent} />
        ) : (
          <VarDisplay
            variable={{ key: '', value: '', source: '' }}
            gameEvent={gameEvent}
          />
        )}
      </div>
    </div>
  );
}
