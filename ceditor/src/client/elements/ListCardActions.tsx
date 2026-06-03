import { Button } from './Button';

export interface ListCardActionsProps {
  onClone: () => void;
  onDelete: () => void;
}

export function ListCardActions({ onClone, onDelete }: ListCardActionsProps) {
  return (
    <>
      <Button variant="small" onClick={onClone} className="btn-card-action" ariaLabel="Clone">
        C
      </Button>
      <Button variant="small" onClick={onDelete} className="btn-danger btn-card-action" ariaLabel="Delete">
        X
      </Button>
    </>
  );
}
