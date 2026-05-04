import { render } from 'preact';

const ButtonsDemo = () => (
    <div style={{
        width: '100%',
        height: '100%',
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#f5f5f5'
    }}>
        <div style={{
            width: '520px',
            padding: '40px 16px',
            backgroundColor: 'white',
            borderRadius: '8px',
            boxShadow: '0 2px 8px rgba(0, 0, 0, 0.1)'
        }}>
            <h1 style={{
                fontSize: '28px',
                fontWeight: 600,
                marginBottom: '32px',
                color: '#1a1a1a'
            }}>
                Button Variants
            </h1>

            {/* Primary Buttons */}
            <div style={{ marginBottom: '32px' }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '16px',
                    color: '#666'
                }}>
                    Primary Buttons
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '8px',
                    flexWrap: 'wrap'
                }}>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#0066cc',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Primary
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#0066cc',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer',
                        opacity: 0.6
                    }}>
                        Disabled
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#0052a3',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Hover
                    </button>
                </div>
            </div>

            {/* Secondary Buttons */}
            <div style={{ marginBottom: '32px' }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '16px',
                    color: '#666'
                }}>
                    Secondary Buttons
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '8px',
                    flexWrap: 'wrap'
                }}>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: 'white',
                        color: '#0066cc',
                        border: '1px solid #0066cc',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Secondary
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: 'white',
                        color: '#0066cc',
                        border: '1px solid #0066cc',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer',
                        opacity: 0.6
                    }}>
                        Disabled
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#f0f6ff',
                        color: '#0066cc',
                        border: '1px solid #0066cc',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Hover
                    </button>
                </div>
            </div>

            {/* Danger Buttons */}
            <div style={{ marginBottom: '32px' }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '16px',
                    color: '#666'
                }}>
                    Danger Buttons
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '8px',
                    flexWrap: 'wrap'
                }}>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#d32f2f',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Delete
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#d32f2f',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer',
                        opacity: 0.6
                    }}>
                        Disabled
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#b71c1c',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Hover
                    </button>
                </div>
            </div>

            {/* Success Buttons */}
            <div style={{ marginBottom: '32px' }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '16px',
                    color: '#666'
                }}>
                    Success Buttons
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '8px',
                    flexWrap: 'wrap'
                }}>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#4caf50',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Confirm
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#4caf50',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer',
                        opacity: 0.6
                    }}>
                        Disabled
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#388e3c',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Hover
                    </button>
                </div>
            </div>

            {/* Size Variants */}
            <div style={{ marginBottom: '32px' }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '16px',
                    color: '#666'
                }}>
                    Size Variants
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '8px',
                    flexWrap: 'wrap',
                    alignItems: 'center'
                }}>
                    <button style={{
                        padding: '4px 8px',
                        backgroundColor: '#0066cc',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '12px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Small
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: '#0066cc',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Medium
                    </button>
                    <button style={{
                        padding: '12px 24px',
                        backgroundColor: '#0066cc',
                        color: 'white',
                        border: 'none',
                        borderRadius: '4px',
                        fontSize: '16px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Large
                    </button>
                </div>
            </div>

            {/* Ghost Buttons */}
            <div>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '16px',
                    color: '#666'
                }}>
                    Ghost Buttons
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '8px',
                    flexWrap: 'wrap'
                }}>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: 'transparent',
                        color: '#0066cc',
                        border: '1px solid transparent',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Ghost
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: 'transparent',
                        color: '#0066cc',
                        border: '1px solid transparent',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer',
                        opacity: 0.6
                    }}>
                        Disabled
                    </button>
                    <button style={{
                        padding: '10px 16px',
                        backgroundColor: 'rgba(0, 102, 204, 0.1)',
                        color: '#0066cc',
                        border: '1px solid transparent',
                        borderRadius: '4px',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer'
                    }}>
                        Hover
                    </button>
                </div>
            </div>
        </div>
    </div>
);

render(<ButtonsDemo />, document.getElementById('root'));
