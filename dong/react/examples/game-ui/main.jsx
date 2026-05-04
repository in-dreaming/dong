import React, { useState, useEffect, useCallback } from 'react';
import { render } from '../../src/dong-react-renderer';
import '../../src/polyfills';

function HealthBar({ current, max, label }) {
    const pct = Math.max(0, Math.min(100, (current / max) * 100));
    const barColor = pct > 60 ? '#27ae60' : pct > 30 ? '#f39c12' : '#e74c3c';

    return (
        <div style={{ marginBottom: '8px' }}>
            <div style={{
                display: 'flex',
                justifyContent: 'space-between',
                fontSize: '12px',
                color: '#bdc3c7',
                marginBottom: '4px',
            }}>
                <span>{label}</span>
                <span>{current}/{max}</span>
            </div>
            <div style={{
                width: '200px',
                height: '8px',
                backgroundColor: '#2c3e50',
                borderRadius: '4px',
                overflow: 'hidden',
            }}>
                <div style={{
                    width: pct + '%',
                    height: '100%',
                    backgroundColor: barColor,
                    borderRadius: '4px',
                }} />
            </div>
        </div>
    );
}

function ScoreDisplay({ score, highScore }) {
    return (
        <div style={{
            textAlign: 'right',
            padding: '10px',
        }}>
            <div style={{ fontSize: '14px', color: '#95a5a6' }}>SCORE</div>
            <div style={{ fontSize: '28px', fontWeight: 'bold', color: '#ecf0f1' }}>
                {score.toLocaleString()}
            </div>
            <div style={{ fontSize: '11px', color: '#7f8c8d' }}>
                HI: {highScore.toLocaleString()}
            </div>
        </div>
    );
}

function Minimap() {
    return (
        <div style={{
            width: '120px',
            height: '120px',
            backgroundColor: '#1a252f',
            border: '2px solid #34495e',
            borderRadius: '4px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            position: 'relative',
        }}>
            <div style={{
                width: '6px',
                height: '6px',
                backgroundColor: '#3498db',
                borderRadius: '50%',
            }} />
            <div style={{
                position: 'absolute',
                bottom: '2px',
                right: '4px',
                fontSize: '9px',
                color: '#7f8c8d',
            }}>MAP</div>
        </div>
    );
}

function InventorySlot({ item, index }) {
    return (
        <div style={{
            width: '48px',
            height: '48px',
            backgroundColor: item ? '#2c3e50' : '#1a252f',
            border: '1px solid #34495e',
            borderRadius: '4px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            fontSize: '20px',
            position: 'relative',
        }}>
            {item ? item.icon : ''}
            {item && item.count > 1 && (
                <span style={{
                    position: 'absolute',
                    bottom: '1px',
                    right: '3px',
                    fontSize: '10px',
                    color: '#bdc3c7',
                }}>{item.count}</span>
            )}
            <span style={{
                position: 'absolute',
                top: '1px',
                left: '3px',
                fontSize: '8px',
                color: '#7f8c8d',
            }}>{index + 1}</span>
        </div>
    );
}

function Hotbar({ items }) {
    const slots = [];
    for (let i = 0; i < 8; i++) {
        slots.push(<InventorySlot key={i} item={items[i] || null} index={i} />);
    }
    return (
        <div style={{ display: 'flex', gap: '4px' }}>
            {slots}
        </div>
    );
}

function GameHUD() {
    const [hp, setHp] = useState(85);
    const [mp, setMp] = useState(60);
    const [score, setScore] = useState(12750);

    const items = [
        { icon: '\u2694', count: 1 },
        { icon: '\u{1F6E1}', count: 1 },
        { icon: '\u2764', count: 3 },
        { icon: '\u2B50', count: 7 },
        null, null, null, null,
    ];

    useEffect(() => {
        const id = setInterval(() => {
            setScore(s => s + Math.floor(Math.random() * 10));
        }, 1000);
        return () => clearInterval(id);
    }, []);

    const takeDamage = useCallback(() => {
        setHp(h => Math.max(0, h - 10));
    }, []);

    const heal = useCallback(() => {
        setHp(h => Math.min(100, h + 15));
    }, []);

    return (
        <div style={{
            width: '100%',
            height: '100%',
            fontFamily: 'sans-serif',
            color: '#ecf0f1',
            position: 'relative',
        }}>
            {/* Top-left: Health/Mana bars */}
            <div style={{
                position: 'absolute',
                top: '15px',
                left: '15px',
            }}>
                <HealthBar current={hp} max={100} label="HP" />
                <HealthBar current={mp} max={100} label="MP" />
                <div style={{ display: 'flex', gap: '6px', marginTop: '8px' }}>
                    <button onClick={takeDamage} style={{
                        padding: '4px 10px', fontSize: '11px',
                        backgroundColor: '#e74c3c', color: 'white',
                        border: 'none', borderRadius: '3px', cursor: 'pointer',
                    }}>Damage</button>
                    <button onClick={heal} style={{
                        padding: '4px 10px', fontSize: '11px',
                        backgroundColor: '#27ae60', color: 'white',
                        border: 'none', borderRadius: '3px', cursor: 'pointer',
                    }}>Heal</button>
                </div>
            </div>

            {/* Top-right: Score */}
            <div style={{ position: 'absolute', top: '15px', right: '15px' }}>
                <ScoreDisplay score={score} highScore={50000} />
            </div>

            {/* Bottom-right: Minimap */}
            <div style={{ position: 'absolute', bottom: '15px', right: '15px' }}>
                <Minimap />
            </div>

            {/* Bottom-center: Hotbar */}
            <div style={{
                position: 'absolute',
                bottom: '15px',
                left: '50%',
                marginLeft: '-208px',
            }}>
                <Hotbar items={items} />
            </div>
        </div>
    );
}

render(<GameHUD />, document.getElementById('root'));
