import { h } from 'preact';

const colorMap = {
    blue:   { bg: '#ebf5fb', color: '#2980b9' },
    green:  { bg: '#eafaf1', color: '#27ae60' },
    red:    { bg: '#fdedec', color: '#e74c3c' },
    yellow: { bg: '#fef9e7', color: '#f39c12' },
    gray:   { bg: '#f2f3f4', color: '#7f8c8d' },
};

export function Badge({ children, color = 'blue', style }) {
    const c = colorMap[color] || colorMap.blue;
    return (
        <span style={{
            display: 'inline-block',
            padding: '2px 8px',
            borderRadius: '10px',
            fontSize: '12px',
            fontWeight: 'bold',
            backgroundColor: c.bg,
            color: c.color,
            ...style,
        }}>
            {children}
        </span>
    );
}
