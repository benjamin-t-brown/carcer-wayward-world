import { AbilityDeleteConfirmModal } from './AbilityDeleteConfirmModal';
import { AbilityDeleteImpact } from '../types/assets';

interface AbilityAttackDeleteModalProps {
  isOpen: boolean;
  attackNumber: number;
  abilityName: string;
  impacts: AbilityDeleteImpact[];
  onConfirm: () => void;
  onCancel: () => void;
}

export function AbilityAttackDeleteModal({
  isOpen,
  attackNumber,
  abilityName,
  impacts,
  onConfirm,
  onCancel,
}: AbilityAttackDeleteModalProps) {
  return (
    <AbilityDeleteConfirmModal
      isOpen={isOpen}
      title={`Delete attack ${attackNumber}?`}
      description={
        <>
          This will remove attack {attackNumber} from ability{' '}
          <strong style={{ color: '#e0e0e0' }}>{abilityName}</strong> and update{' '}
          {impacts.length === 1 ? '1 asset' : `${impacts.length} assets`}:
        </>
      }
      impacts={impacts}
      confirmLabel="Delete attack & update assets"
      onConfirm={onConfirm}
      onCancel={onCancel}
    />
  );
}
