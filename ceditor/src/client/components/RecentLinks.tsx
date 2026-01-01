interface RecentLinksProps {
  items: {
    label: string;
    isSelected: boolean;
    onClick: () => void;
    onRemove: () => void;
  }[];
}

export const RecentLinks = ({ items }: RecentLinksProps) => {
  if (items.length === 0) {
    return null;
  }
  return (
    <div
      style={{
        fontSize: 12,
        background: '#252526',
        borderRadius: 4,
        padding: 4,
        lineHeight: '20px',
        cursor: 'pointer',
      }}
    >
      {items.map((item, index) => (
        <div
          style={{
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'center',
            gap: 8,
          }}
          key={item.label + index}
          onClick={item.onClick}
        >
          <span
            style={{ textDecoration: item.isSelected ? 'underline' : 'none' }}
          >
            {item.label}
          </span>
          <span
            role="img"
            aria-label="Remove"
            style={{ marginLeft: 8, cursor: 'pointer', filter: 'sepia(1)' }}
            onClick={(ev) => {
              ev.stopPropagation();
              item.onRemove();
            }}
          >
            ❌
          </span>
        </div>))}
    </div>
  );
};
