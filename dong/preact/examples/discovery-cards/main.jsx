import { render } from 'preact';

const CardsDemo = () => (
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
            backgroundColor: 'transparent'
        }}>
            <h1 style={{
                fontSize: '28px',
                fontWeight: 600,
                marginBottom: '32px',
                color: '#1a1a1a'
            }}>
                Card Components
            </h1>

            {/* Card 1: User Profile */}
            <div style={{
                marginBottom: '24px',
                backgroundColor: 'white',
                borderRadius: '8px',
                padding: '16px',
                boxShadow: '0 2px 8px rgba(0, 0, 0, 0.1)',
                border: '1px solid #e0e0e0'
            }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '12px',
                    color: '#333'
                }}>
                    User Profile
                </h2>
                <div style={{
                    display: 'flex',
                    gap: '12px',
                    alignItems: 'center'
                }}>
                    <div style={{
                        width: '48px',
                        height: '48px',
                        borderRadius: '50%',
                        backgroundColor: '#0066cc',
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                        color: 'white',
                        fontSize: '20px',
                        fontWeight: 600
                    }}>
                        JD
                    </div>
                    <div>
                        <div style={{
                            fontSize: '18px',
                            fontWeight: 600,
                            color: '#1a1a1a'
                        }}>
                            John Doe
                        </div>
                        <div style={{
                            fontSize: '12px',
                            color: '#666'
                        }}>
                            Senior Developer
                        </div>
                        <div style={{
                            display: 'flex',
                            gap: '4px',
                            marginTop: '4px'
                        }}>
                            <span style={{
                                display: 'inline-block',
                                fontSize: '11px',
                                padding: '2px 6px',
                                backgroundColor: '#e3f2fd',
                                color: '#0066cc',
                                borderRadius: '3px'
                            }}>
                                Preact
                            </span>
                            <span style={{
                                display: 'inline-block',
                                fontSize: '11px',
                                padding: '2px 6px',
                                backgroundColor: '#f1f8e9',
                                color: '#558b2f',
                                borderRadius: '3px'
                            }}>
                                C++
                            </span>
                            <span style={{
                                display: 'inline-block',
                                fontSize: '11px',
                                padding: '2px 6px',
                                backgroundColor: '#fff8e1',
                                color: '#f57f17',
                                borderRadius: '3px'
                            }}>
                                GPU
                            </span>
                        </div>
                    </div>
                </div>
            </div>

            {/* Card 2: Project Stats */}
            <div style={{
                marginBottom: '24px',
                backgroundColor: 'white',
                borderRadius: '8px',
                padding: '16px',
                boxShadow: '0 2px 8px rgba(0, 0, 0, 0.1)',
                border: '1px solid #e0e0e0'
            }}>
                <h2 style={{
                    fontSize: '16px',
                    fontWeight: 600,
                    marginBottom: '12px',
                    color: '#333'
                }}>
                    Project Stats
                </h2>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
                    <div style={{
                        display: 'flex',
                        justifyContent: 'space-between',
                        alignItems: 'center',
                        paddingBottom: '8px',
                        borderBottom: '1px solid #f0f0f0'
                    }}>
                        <span style={{ fontSize: '14px', color: '#333' }}>Commits</span>
                        <span style={{
                            display: 'inline-block',
                            fontSize: '12px',
                            padding: '4px 8px',
                            backgroundColor: '#f1f8e9',
                            color: '#558b2f',
                            borderRadius: '3px',
                            fontWeight: 600
                        }}>
                            1,247
                        </span>
                    </div>
                    <div style={{
                        display: 'flex',
                        justifyContent: 'space-between',
                        alignItems: 'center',
                        paddingBottom: '8px',
                        borderBottom: '1px solid #f0f0f0'
                    }}>
                        <span style={{ fontSize: '14px', color: '#333' }}>Open Issues</span>
                        <span style={{
                            display: 'inline-block',
                            fontSize: '12px',
                            padding: '4px 8px',
                            backgroundColor: '#fff8e1',
                            color: '#f57f17',
                            borderRadius: '3px',
                            fontWeight: 600
                        }}>
                            23
                        </span>
                    </div>
                    <div style={{
                        display: 'flex',
                        justifyContent: 'space-between',
                        alignItems: 'center'
                    }}>
                        <span style={{ fontSize: '14px', color: '#333' }}>Contributors</span>
                        <span style={{
                            display: 'inline-block',
                            fontSize: '12px',
                            padding: '4px 8px',
                            backgroundColor: '#e3f2fd',
                            color: '#0066cc',
                            borderRadius: '3px',
                            fontWeight: 600
                        }}>
                            8
                        </span>
                    </div>
                </div>
            </div>

            {/* Card 3: Empty State */}
            <div style={{
                backgroundColor: 'white',
                borderRadius: '8px',
                padding: '16px',
                boxShadow: '0 2px 8px rgba(0, 0, 0, 0.1)',
                border: '1px solid #e0e0e0',
                textAlign: 'center'
            }}>
                <p style={{
                    fontSize: '12px',
                    color: '#999',
                    margin: '0'
                }}>
                    Card without title — just content
                </p>
            </div>
        </div>
    </div>
);

render(<CardsDemo />, document.getElementById('root'));
