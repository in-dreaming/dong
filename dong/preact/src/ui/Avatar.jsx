import { h } from 'preact';

const sizeMap = {
    sm: 28,
    md: 40,
    lg: 56,
};

const bgColors = ['#3498db', '#e74c3c', '#27ae60', '#f39c12', '#9b59b6', '#1abc9c', '#e67e22'];

function getInitials(name) {
    if (!name) return '?';
    const parts = name.split(' ');
    if (parts.length >= 2) return (parts[0][0] + parts[1][0]).toUpperCase();
    return name.slice(0, 2).toUpperCase();
}

function hashColor(name) {
    let h = 0;
    for (let i = 0; i < (name || '').length; i++) h = (h * 31 + name.charCodeAt(i)) | 0;
    return bgColors[Math.abs(h) % bgColors.length];
}

export function Avatar({ name, size = 'md', style }) {
    const px = sizeMap[size] || sizeMap.md;
    return (
        <div style={{
            width: px + 'px',
            height: px + 'px',
            borderRadius: '50%',
            backgroundColor: hashColor(name),
            color: '#fff',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            fontSize: (px * 0.4) + 'px',
            fontWeight: 'bold',
            flexShrink: '0',
            ...style,
        }}>
            {getInitials(name)}
        </div>
    );
}
