import { h } from 'preact';

const presets = {
    h1:      { fontSize: '28px', fontWeight: 'bold', color: '#2c3e50' },
    h2:      { fontSize: '22px', fontWeight: 'bold', color: '#2c3e50' },
    h3:      { fontSize: '18px', fontWeight: 'bold', color: '#2c3e50' },
    body:    { fontSize: '14px', fontWeight: 'normal', color: '#34495e' },
    caption: { fontSize: '12px', fontWeight: 'normal', color: '#95a5a6' },
    label:   { fontSize: '13px', fontWeight: 'bold', color: '#7f8c8d' },
};

export function Text({ children, variant = 'body', color, align, style, ...props }) {
    const p = presets[variant] || presets.body;
    const tag = variant.startsWith('h') ? variant : 'span';
    return h(tag, {
        style: {
            ...p,
            color: color || p.color,
            textAlign: align,
            ...style,
        },
        ...props,
    }, children);
}
