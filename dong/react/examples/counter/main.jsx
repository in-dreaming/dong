import React, { useState, useCallback } from 'react';
import { render } from '../../src/dong-react-renderer';
import '../../src/polyfills';

function Counter() {
    const [count, setCount] = useState(0);
    const increment = useCallback(() => setCount(c => c + 1), []);
    const decrement = useCallback(() => setCount(c => c - 1), []);
    const reset = useCallback(() => setCount(0), []);

    return (
        <div style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            justifyContent: 'center',
            padding: '40px',
            fontFamily: 'sans-serif',
        }}>
            <h1 style={{ color: '#2c3e50', fontSize: '32px', marginBottom: '20px' }}>
                React + Dong Counter
            </h1>
            <div style={{
                fontSize: '64px',
                fontWeight: 'bold',
                color: count >= 0 ? '#27ae60' : '#e74c3c',
                marginBottom: '30px',
            }}>
                {count}
            </div>
            <div style={{ display: 'flex', gap: '10px' }}>
                <Button onClick={decrement} color="#e74c3c">-</Button>
                <Button onClick={reset} color="#95a5a6">Reset</Button>
                <Button onClick={increment} color="#27ae60">+</Button>
            </div>
        </div>
    );
}

function Button({ onClick, color, children }) {
    return (
        <button
            onClick={onClick}
            style={{
                padding: '12px 24px',
                backgroundColor: color,
                color: 'white',
                border: 'none',
                borderRadius: '6px',
                fontSize: '18px',
                cursor: 'pointer',
                minWidth: '60px',
            }}
        >
            {children}
        </button>
    );
}

function App() {
    return (
        <div style={{
            width: '100%',
            height: '100%',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            backgroundColor: '#ecf0f1',
        }}>
            <Counter />
        </div>
    );
}

render(<App />, document.getElementById('root'));
