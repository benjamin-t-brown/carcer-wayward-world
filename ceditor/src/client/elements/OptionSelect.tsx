import React from 'react';

interface OptionSelectProps {
  id?: string;
  name?: string;
  label?: string;
  value: string;
  onChange: (value: string) => void;
  options: Array<{ value: string; label: string }>;
  required?: boolean;
  disabled?: boolean;
}

export const OptionSelect: React.FC<OptionSelectProps> = ({
  id,
  name,
  label,
  value,
  onChange,
  options,
  required = false,
  disabled = false,
}) => {
  return (
    <div className="form-group">
      {label && (
        <label htmlFor={id}>
          {label}
          {required && ' *'}
        </label>
      )}
      <select
        id={id}
        name={name}
        value={value}
        onChange={(e) => onChange(e.target.value)}
        required={required}
        disabled={disabled}
      >
        {options.map((option) => (
          <option key={option.value} value={option.value}>
            {option.label}
          </option>
        ))}
      </select>
    </div>
  );
};

