interface ModalRowLayoutProps {
  fixedHeight: number;
  fixedContent: React.ReactNode;
  resizableContent: React.ReactNode;
}
export const ModalRowLayout = (props: ModalRowLayoutProps) => {
  return (
    <div
      style={{
        width: '100%',
        height: '100%',
      }}
    >
      <div style={{ height: `${props.fixedHeight}px` }}>
        {props.fixedContent}
      </div>
      <div style={{ height: `calc(100% - ${props.fixedHeight}px)` }}>
        {props.resizableContent}
      </div>
    </div>
  );
};
