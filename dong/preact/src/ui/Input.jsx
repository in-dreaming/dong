import { h } from 'preact';
import { useState } from 'preact/hooks';

export function Input({ label, placeholder, value, onInput, type = 'text', disabled, style, ...props }) {
    const [focused, setFocused] = useState(false);

    return (
        <div style={{ marginBottom: '12px', ...style }}>
            {label && (
                <div style={{
                    fontSize: '13px',
                    fontWeight: 'bold',
                    color: '#7f8c8d',
                    marginBottom: '4px',
                }}>
                    {label}
                </div>
            )}
            <input
                type={type}
                placeholder={placeholder}
                value={value}
                onInput={onInput}
                disabled={disabled}
                onFocus={() => setFocused(true)}
                onBlur={() => setFocused(false)}
                style={{
                    width: '100%',
                    padding: '10px 12px',
                    fontSize: '14px',
                    border: focused ? '2px solid #3498db' : '2px solid #dcdde1',
                    borderRadius: '6px',
                    backgroundColor: disabled ? '#f5f6fa' : '#fff',
                    color: disabled ? '#95a5a6' : '#2c3e50',
                }}
                {...props}
            />
        </div>
    );
}
