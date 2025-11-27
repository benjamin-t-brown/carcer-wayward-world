import { useState, useMemo } from 'react';
import { GameEvent, Variable } from '../types/assets';
import { useAssets } from '../contexts/AssetsContext';
import { getVarsFromNode } from './nodeRendering/nodeHelpers';

interface VariableWidgetProps {
  gameEvent: GameEvent | null;
}

export function VariableWidget({ gameEvent }: VariableWidgetProps) {
  const { gameEvents } = useAssets();
  const [copiedVar, setCopiedVar] = useState<string | null>(null);
  const [includeImports, setIncludeImports] = useState<boolean>(false);
  const [isExpanded, setIsExpanded] = useState<boolean>(false);

  const accessibleVars = gameEvent
    ? getVarsFromNode(gameEvent, gameEvents)
    : [];

  const handleCopyVariable = async (variableName: string) => {
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
    <div
      style={{
        backgroundColor: '#252526',
        border: '1px solid #3e3e42',
        marginBottom: '10px',
      }}
    >
      <div
        onClick={() => setIsExpanded(!isExpanded)}
        style={{
          display: 'flex',
          alignItems: 'center',
          gap: '10px',
        }}
      >
        <div
          style={{
            transform: isExpanded ? 'rotate(90deg)' : 'rotate(0deg)',
            padding: '4px',
          }}
        >
          {'>'}
        </div>
        <div>Variables</div>
      </div>
      <div
        style={{
          display: isExpanded ? 'flex' : 'none',
          alignItems: 'center',
          gap: '10px',
        }}
      >
        <input
          id="include-imports-checkbox"
          type="checkbox"
          checked={includeImports}
          onChange={() => setIncludeImports(!includeImports)}
        />
        <label htmlFor="include-imports-checkbox">Include imports</label>
      </div>
      {accessibleVars.length === 0 ? (
        <div
          style={{
            color: '#858585',
            fontSize: '12px',
            fontStyle: 'italic',
          }}
        >
          No variables available
        </div>
      ) : (
        <div
          style={{
            display: isExpanded ? 'flex' : 'none',
            flexDirection: 'column',
            gap: '5px',
            overflowY: 'auto',
            maxHeight: '200px',
          }}
        >
          {accessibleVars
            .filter(
              (variable) => includeImports || variable.source === gameEvent.id
            )
            .map((variable, index) => (
              <div
                key={`${variable.key}-${variable.source}-${index}`}
                onClick={() => handleCopyVariable(variable.key)}
                style={{
                  cursor: 'pointer',
                }}
              >
                <span
                  style={{
                    color:
                      variable.source === gameEvent.id ? '#4ec9b0' : 'orange',
                    fontSize: '13px',
                    fontWeight: '500',
                    textDecoration: 'underline',
                  }}
                >
                  {variable.key}
                </span>
                <span
                  style={{
                    color: '#858585',
                    fontSize: '11px',
                  }}
                >
                  = {variable.value}
                </span>
                {variable.source !== gameEvent.id && (
                  <span
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      fontStyle: 'italic',
                      marginLeft: '10px',
                    }}
                  >
                    ({variable.source})
                  </span>
                )}
                {copiedVar === variable.key && (
                  <span
                    style={{
                      color: '#4ec9b0',
                      fontSize: '11px',
                      marginLeft: '10px',
                      fontWeight: '500',
                    }}
                  >
                    ✓ Copied! {variable.key}
                  </span>
                )}
              </div>
            ))}
        </div>
      )}
    </div>
  );
}
