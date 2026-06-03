interface ModalRowLayoutProps {
  sidebar: React.ReactNode;
  main: React.ReactNode;
  className?: string;
}

export const ModalRowLayout = (props: ModalRowLayoutProps) => {
  return (
    <div
      className={props.className}
      style={{
        display: 'flex',
        gap: '16px',
        flex: 1,
        minHeight: 0,
        height: '100%',
      }}
      onClick={(e) => e.stopPropagation()}
    >
      <div
        style={{
          flexShrink: 0,
          width: '220px',
          overflowY: 'auto',
          maxHeight: '100%',
        }}
      >
        {props.sidebar}
      </div>
      <div
        style={{
          flex: 1,
          minWidth: 0,
          minHeight: 0,
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}
      >
        {props.main}
      </div>
    </div>
  );
};
