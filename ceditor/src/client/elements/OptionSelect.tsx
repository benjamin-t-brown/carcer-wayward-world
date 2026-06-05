import React from 'react';

interface OptionSelectProps {
  id?: string;
  name?: string;
  label?: string;
  value: string;
  onChange: (value: string) => void;
  options: Array<{ value: string | number; label: string }>;
  required?: boolean;
  disabled?: boolean;
  style?: React.CSSProperties;
  className?: string;
  inputStyle?: React.CSSProperties;
}

export const OptionSelect: React.FC<OptionSelectProps> = ({
  id,
  name,
  label,
  value,
  onChange,
  options,
  style = {},
  inputStyle = {},
  required = false,
  disabled = false,
  className = '',
}) => {
  const groupClass = `form-group ${className}`.trim();

  return (
    <div className={groupClass} style={style}>
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
        style={inputStyle}
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

