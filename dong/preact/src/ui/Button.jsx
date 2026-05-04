import { h } from 'preact';
import { useState } from 'preact/hooks';

const variants = {
    primary:   { bg: '#3498db', color: '#fff', border: 'none' },
    secondary: { bg: '#ecf0f1', color: '#2c3e50', border: '1px solid #bdc3c7' },
    danger:    { bg: '#e74c3c', color: '#fff', border: 'none' },
    success:   { bg: '#27ae60', color: '#fff', border: 'none' },
    ghost:     { bg: 'transparent', color: '#3498db', border: '1px solid #3498db' },
};

const sizes = {
    sm: { padding: '6px 12px', fontSize: '12px', borderRadius: '4px' },
    md: { padding: '10px 20px', fontSize: '14px', borderRadius: '6px' },
    lg: { padding: '14px 28px', fontSize: '16px', borderRadius: '8px' },
};

export function Button({ children, variant = 'primary', size = 'md', disabled, block, style, ...props }) {
    const [hover, setHover] = useState(false);
    const v = variants[variant] || variants.primary;
    const s = sizes[size] || sizes.md;

    const baseStyle = {
        ...s,
        backgroundColor: disabled ? '#bdc3c7' : v.bg,
        color: disabled ? '#7f8c8d' : v.color,
        border: v.border,
        cursor: disabled ? 'default' : 'pointer',
        fontWeight: 'bold',
        display: block ? 'block' : 'inline-block',
        width: block ? '100%' : 'auto',
        textAlign: 'center',
        opacity: hover && !disabled ? '0.85' : '1',
        ...style,
    };

    return (
        <button
            style={baseStyle}
            disabled={disabled}
            onMouseEnter={() => setHover(true)}
            onMouseLeave={() => setHover(false)}
            {...props}
        >
            {children}
        </button>
    );
}
