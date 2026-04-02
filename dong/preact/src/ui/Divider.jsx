import { h } from 'preact';

export function Divider({ style }) {
    return (
        <div style={{
            height: '1px',
            backgroundColor: '#ecf0f1',
            margin: '12px 0',
            ...style,
        }} />
    );
}
