import { h } from 'preact';

export function Card({ children, title, padding = '16px', style, ...props }) {
    return (
        <div
            style={{
                backgroundColor: '#fff',
                borderRadius: '8px',
                boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
                overflow: 'hidden',
                ...style,
            }}
            {...props}
        >
            {title && (
                <div style={{
                    padding: '12px 16px',
                    borderBottom: '1px solid #ecf0f1',
                    fontWeight: 'bold',
                    fontSize: '16px',
                    color: '#2c3e50',
                }}>
                    {title}
                </div>
            )}
            <div style={{ padding }}>
                {children}
            </div>
        </div>
    );
}
