import { useState } from 'react';
import { GameEvent, Variable } from '../../types/assets';
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
        <span style={{ padding: '4px' }}>No variable selected</span>
      )}
    </div>
  );
};

export function VariableWidget({ gameEvent }: VariableWidgetProps) {
  const { gameEvents } = useAssets();
  const [copiedVar, setCopiedVar] = useState<string | null>(null);
  const [hoverVar, setHoverVar] = useState<AccessibleVariable | null>(null);
  // const [includeImports, setIncludeImports] = useState<boolean>(false);
  // const [isExpanded, setIsExpanded] = useState<boolean>(false);

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
    <div>
      <div>Search Variables</div>
      <SearchInput
        items={accessibleVars}
        onSelect={(variable) => {
          console.log('select', variable);
          handleCopyVariable('@' + variable.key);
          setCopiedVar('@' + variable.key);
          setHoverVar(variable);
        }}
        searchFields={(variable: AccessibleVariable) => [variable.key]}
        renderItem={(variable) => (
          <VarDisplay variable={variable} gameEvent={gameEvent} />
        )}
        getItemKey={(variable) => variable.key}
      />
      <div>
        {hoverVar ? (
          <VarDisplay variable={hoverVar} gameEvent={gameEvent} />
        ) : (
          <VarDisplay
            variable={{ key: '', value: '', source: '' }}
            gameEvent={gameEvent}
          />
        )}
        {/* {copiedVar && (
          <span
            style={{
              color: '#4ec9b0',
              fontSize: '11px',
              marginLeft: '10px',
              fontWeight: '500',
            }}
          >
            ✓ Copied! {copiedVar}
          </span>
        )} */}
      </div>
    </div>
  );
  // return (
  //   <div
  //     style={{
  //       backgroundColor: '#252526',
  //       border: '1px solid #3e3e42',
  //       marginBottom: '10px',
  //     }}
  //   >
  //     <div
  //       style={{
  //         display: 'flex',
  //         alignItems: 'center',
  //         gap: '10px',
  //         cursor: 'pointer',
  //         userSelect: 'none',
  //         background: '#555',
  //       }}
  //     >
  //       <div
  //         style={{
  //           marginLeft: '4px',
  //         }}
  //       >
  //         Variables
  //       </div>
  //       <div
  //         style={{
  //           display: 'flex',
  //           alignItems: 'center',
  //           gap: '10px',
  //           padding: '4px',
  //         }}
  //       >
  //         <input
  //           id="include-imports-checkbox"
  //           type="checkbox"
  //           checked={includeImports}
  //           onChange={() => setIncludeImports(!includeImports)}
  //         />
  //         <label
  //           htmlFor="include-imports-checkbox"
  //           style={{ fontSize: '12px' }}
  //         >
  //           Include imports
  //         </label>
  //       </div>
  //       {hoverVar && (
  //         <div
  //           style={{
  //             color: '#d5d5d5',
  //             fontSize: '12px',
  //           }}
  //         >
  //           {hoverVar.key} = {hoverVar.value}
  //           {hoverVar.source !== gameEvent.id && (
  //             <span
  //               style={{
  //                 color: '#a5a5a5',
  //                 marginLeft: '10px',
  //               }}
  //             >
  //               ({hoverVar.source})
  //             </span>
  //           )}
  //         </div>
  //       )}
  //       {copiedVar && (
  //         <span
  //           style={{
  //             color: '#4ec9b0',
  //             fontSize: '11px',
  //             marginLeft: '10px',
  //             fontWeight: '500',
  //           }}
  //         >
  //           ✓ Copied! {copiedVar}
  //         </span>
  //       )}
  //     </div>
  //     {accessibleVars.length === 0 ? (
  //       <div
  //         style={{
  //           color: '#858585',
  //           fontSize: '12px',
  //           fontStyle: 'italic',
  //           padding: '16px',
  //         }}
  //       >
  //         No variables available
  //       </div>
  //     ) : (
  //       <div
  //         style={{
  //           display: 'flex',
  //           flexWrap: 'wrap',
  //           gap: '5px',
  //           overflowY: 'auto',
  //           // maxHeight: '60px',
  //           margin: '4px',
  //         }}
  //       >
  //         {accessibleVars
  //           .filter(
  //             (variable) => includeImports || variable.source === gameEvent.id
  //           )
  //           .map((variable, index) => (
  //             <div
  //               key={`${variable.key}-${variable.source}-${index}`}
  //               onClick={() => handleCopyVariable(variable.key)}
  //               onMouseEnter={() => setHoverVar(variable)}
  //               style={{
  //                 cursor: 'pointer',
  //                 background: '#444',
  //                 borderRadius: '8px',
  //                 padding: '4px',
  //               }}
  //             >
  //               <span
  //                 style={{
  //                   color:
  //                     variable.source === gameEvent.id ? '#4ec9b0' : 'orange',
  //                   fontSize: '13px',
  //                   fontWeight: '500',
  //                   textDecoration: 'underline',
  //                 }}
  //               >
  //                 @{variable.key}
  //               </span>
  //             </div>
  //           ))}
  //       </div>
  //     )}
  //   </div>
  // );
}
