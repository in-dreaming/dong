import { h } from 'preact';

export function Stack({ children, direction = 'column', gap = '8px', align, justify, style, ...props }) {
    return (
        <div
            style={{
                display: 'flex',
                flexDirection: direction,
                gap,
                alignItems: align,
                justifyContent: justify,
                ...style,
            }}
            {...props}
        >
            {children}
        </div>
    );
}
