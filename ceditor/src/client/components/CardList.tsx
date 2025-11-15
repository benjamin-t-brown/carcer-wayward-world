import { Card } from '../elements/Card';
import { Button } from '../elements/Button';

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
}

export function CardList<T extends ListItem>({
  items,
  onItemClick,
  onClone,
  onDelete,
  selectedIndex = null,
  renderAdditionalInfo,
  emptyMessage = 'No items found',
}: CardListProps<T>) {
  if (items.length === 0) {
    return <div className="empty-state">{emptyMessage}</div>;
  }

  return (
    <div className="item-list">
      {items.map((item, index) => {
        const isSelected = selectedIndex === index;
        const displayLabel = item.label || item.name || 'Unnamed';

        return (
          <Card
            key={index}
            id={`item-card-${index}`}
            variant="item"
            className={isSelected ? 'editing' : ''}
            onClick={() => onItemClick(index)}
          >
            <div className="item-card-header">
              <h3>{displayLabel}</h3>
              <div
                className="item-card-actions"
                onClick={(e) => e.stopPropagation()}
              >
                <Button
                  variant="small"
                  onClick={() => onClone(index)}
                >
                  Clone
                </Button>
                <Button
                  variant="small"
                  onClick={() => onDelete(index)}
                  className="btn-danger"
                >
                  Delete
                </Button>
              </div>
            </div>
            <div className="item-card-body">
              <div className="item-info">
                <span className="item-name">{item.name}</span>
              </div>
              {renderAdditionalInfo && renderAdditionalInfo(item)}
            </div>
          </Card>
        );
      })}
    </div>
  );
}

