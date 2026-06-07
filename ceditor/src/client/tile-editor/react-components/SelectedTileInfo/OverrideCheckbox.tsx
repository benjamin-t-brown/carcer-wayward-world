export function OverrideCheckbox({
  value,
  label,
  onChange,
}: {
  value: boolean | undefined;
  label: string;
  onChange: (newValue: boolean | undefined) => void;
}) {
  // Treat undefined as false for display and toggle purposes
  const isChecked = value === true;

  const handleChange = () => {
    // Toggle between true and false only (treating undefined as false)
    const newValue = !isChecked;
    onChange(newValue);
  };

  return (
    <label
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: '8px',
        cursor: 'pointer',
        fontSize: '12px',
      }}
    >
      <input
        type="checkbox"
        checked={isChecked}
        onChange={handleChange}
        style={{
          cursor: 'pointer',
          width: '16px',
          height: '16px',
        }}
      />
      <span style={{ color: '#ffffff' }}>{label}</span>
      <span
        style={{
          color: isChecked ? '#4ec9b0' : '#ff6b6b',
          fontSize: '10px',
          marginLeft: 'auto',
        }}
      >
        {isChecked ? 'true' : 'false'}
      </span>
    </label>
  );
}

