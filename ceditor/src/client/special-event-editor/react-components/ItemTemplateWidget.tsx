import { useState } from 'react';
import { GameEvent, ItemTemplate } from '../../types/assets';
import { useAssets } from '../../contexts/AssetsContext';
import { SearchInput } from '../../elements/SearchInput';

interface ItemTemplateWidgetProps {
  gameEvent: GameEvent | null;
}

const ItemTemplateDisplay = ({
  itemTemplate,
  gameEvent,
}: {
  itemTemplate: ItemTemplate | undefined;
  gameEvent: GameEvent;
}) => {
  return (
    <div
      style={{
        margin: '4px',
        display: 'inline-block',
      }}
    >
      {itemTemplate ? (
        <>
          <span
            style={{
              color: itemTemplate.name === gameEvent.id ? '#4ec9b0' : 'orange',
              backgroundColor: '#252526',
              padding: '4px',
            }}
          >
            {itemTemplate.name}
          </span>{' '}
          ({itemTemplate.label})
        </>
      ) : (
        <span style={{ padding: '0px' }}>No item template selected</span>
      )}
    </div>
  );
};

export function ItemTemplateWidget({ gameEvent }: ItemTemplateWidgetProps) {
  const { gameEvents, items } = useAssets();
  const [copiedItemTemplate, setCopiedItemTemplate] = useState<string | null>(null);
  const [hoverItemTemplate, setHoverItemTemplate] = useState<ItemTemplate | null>(null);
  const [copyTimeoutId, setCopyTimeoutId] = useState<NodeJS.Timeout | null>(
    null
  );
  const handleCopy = async (itemTemplateName: string) => {
    if (copyTimeoutId) {
      clearTimeout(copyTimeoutId);
      setCopyTimeoutId(null);
    }
    try {
      await navigator.clipboard.writeText(itemTemplateName);
      setCopiedItemTemplate(itemTemplateName);
      setTimeout(() => setCopiedItemTemplate(null), 2000);
    } catch (err) {
      console.error('Failed to copy item template name:', err);
    }
  };

  if (!gameEvent) {
    return null;
  }

  return (
    <div>
      <div>
        Search Item Templates{' '}
        <span
          style={{
            color: '#4ec9b0',
            fontSize: '11px',
            marginLeft: '10px',
            fontWeight: '500',
          }}
        >
          {copiedItemTemplate ? `✓ Copied! ${copiedItemTemplate}` : ''}
        </span>
      </div>
      <SearchInput
        items={items}
        onSelect={(itemTemplate: ItemTemplate) => {
          console.log('select', itemTemplate);
          handleCopy(itemTemplate.name);
          setCopiedItemTemplate(itemTemplate.name);
          setHoverItemTemplate(itemTemplate);
        }}
        searchFields={(itemTemplate: ItemTemplate) => [itemTemplate.name]}
        renderItem={(itemTemplate) => (
          <ItemTemplateDisplay
            itemTemplate={itemTemplate}
            gameEvent={gameEvent}
          />
        )}
        getItemKey={(itemTemplate: ItemTemplate) => itemTemplate.name}
      />
      <div
        onClick={() => {
          if (hoverItemTemplate?.name) {
            handleCopy(hoverItemTemplate?.name ?? '');
            setCopiedItemTemplate(hoverItemTemplate?.name ?? '');
          }
        }}
      >
        {hoverItemTemplate ? (
          <ItemTemplateDisplay itemTemplate={hoverItemTemplate} gameEvent={gameEvent} />
        ) : (
          <ItemTemplateDisplay itemTemplate={undefined} gameEvent={gameEvent} />
        )}
      </div>
    </div>
  );
}
