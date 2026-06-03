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
  renderAdditionalInfo?: (item: ListItem) => React.ReactNode;
  emptyMessage?: string;
  getCardId?: (index: number) => string;
}

export function CardList<T extends ListItem>({
  items,
  onItemClick,
  onClone,
  onDelete,
  selectedIndex = null,
  renderAdditionalInfo,
  emptyMessage = 'No items found',
  getCardId = (index) => `item-card-${index}`,
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
            key={index}
            id={getCardId(index)}
            selected={selectedIndex === index}
            onClick={() => onItemClick(index)}
            title={displayLabel}
            subtitle={item.name}
            actions={
              <ListCardActions
                onClone={() => onClone(index)}
                onDelete={() => onDelete(index)}
              />
            }
          >
            {renderAdditionalInfo?.(item)}
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
