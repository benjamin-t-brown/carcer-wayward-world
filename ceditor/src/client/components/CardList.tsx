import type { Key, ReactNode } from 'react';
import { SelectableListCard } from '../elements/SelectableListCard';
import { ListCardActions } from '../elements/ListCardActions';

export interface ListItem {
  label?: string;
  name: string;
}

interface CardListProps<ListItem> {
  items: ListItem[];
  onItemClick: (index: number) => void;
  onClone: (index: number) => void;
  onDelete: (index: number) => void;
  selectedIndex?: number | null;
  renderMedia?: (item: ListItem, index: number) => ReactNode;
  renderAdditionalInfo?: (item: ListItem, index: number) => ReactNode;
  emptyMessage?: string;
  getCardId?: (index: number) => string;
  getItemKey?: (item: ListItem, index: number) => Key;
}

export function CardList<T extends ListItem>({
  items,
  onItemClick,
  onClone,
  onDelete,
  selectedIndex = null,
  renderMedia,
  renderAdditionalInfo,
  emptyMessage = 'No items found',
  getCardId = (index) => `item-card-${index}`,
  getItemKey = (_item, index) => index,
}: CardListProps<T>) {
  if (items.length === 0) {
    return <div className="empty-state">{emptyMessage}</div>;
  }

  return (
    <div className="item-list">
      {items.map((item, index) => {
        const displayLabel = item.label || item.name || 'Unnamed';

        return (
          <SelectableListCard
            key={getItemKey(item, index)}
            id={getCardId(index)}
            selected={selectedIndex === index}
            onClick={() => onItemClick(index)}
            title={displayLabel}
            subtitle={item.name}
            media={renderMedia?.(item, index)}
            actions={
              <ListCardActions
                onClone={() => onClone(index)}
                onDelete={() => onDelete(index)}
              />
            }
          >
            {renderAdditionalInfo?.(item, index)}
          </SelectableListCard>
        );
      })}
    </div>
  );
}

interface CardListAdvancedProps<T> {
  items: T[];
  renderListItem: (item: T, index: number) => React.ReactNode;
  selectedIndex?: number | null;
  onItemClick: (index: number) => void;
  emptyMessage?: string;
  getCardId?: (index: number) => string;
}

export function CardListAdvanced<T>({
  items,
  renderListItem,
  selectedIndex = null,
  onItemClick,
  emptyMessage = 'No items found',
  getCardId = (index) => `item-card-${index}`,
}: CardListAdvancedProps<T>) {
  if (items.length === 0) {
    return <div className="empty-state">{emptyMessage}</div>;
  }

  return (
    <div className="item-list">
      {items.map((item, index) => (
        <SelectableListCard
          key={index}
          id={getCardId(index)}
          selected={selectedIndex === index}
          onClick={() => onItemClick(index)}
        >
          {renderListItem(item, index)}
        </SelectableListCard>
      ))}
    </div>
  );
}
