import { render } from 'preact';

const InboxDemo = () => {
    const messages = [
        { id: 1, from: 'Alice Chen', text: 'Hey, the new build looks great!', time: '2m ago', unread: true },
        { id: 2, from: 'Bob Wang', text: 'Can you review the PR?', time: '15m ago', unread: true },
        { id: 3, from: 'Carol Li', text: 'Meeting at 3pm tomorrow', time: '1h ago', unread: false },
        { id: 4, from: 'Dave Zhang', text: 'Shipped v2.0 to staging', time: '3h ago', unread: false },
    ];

    const unreadCount = messages.filter(m => m.unread).length;

    return (
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
                    Inbox
                </h1>

                {/* Inbox Card */}
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
                        Inbox {unreadCount > 0 ? `(${unreadCount} unread)` : ''}
                    </h2>

                    {/* Search Input */}
                    <input
                        type="text"
                        placeholder="Search messages..."
                        style={{
                            width: '100%',
                            padding: '8px 12px',
                            marginBottom: '12px',
                            border: '1px solid #ddd',
                            borderRadius: '4px',
                            fontSize: '14px',
                            fontFamily: 'inherit'
                        }}
                    />

                    {/* Message List */}
                    <div style={{ display: 'flex', flexDirection: 'column' }}>
                        {messages.map((msg, i) => (
                            <div key={msg.id}>
                                <div style={{
                                    display: 'flex',
                                    gap: '10px',
                                    alignItems: 'center',
                                    padding: '10px 0',
                                    cursor: 'pointer'
                                }}>
                                    {/* Avatar */}
                                    <div style={{
                                        width: '32px',
                                        height: '32px',
                                        borderRadius: '50%',
                                        backgroundColor: '#0066cc',
                                        display: 'flex',
                                        alignItems: 'center',
                                        justifyContent: 'center',
                                        color: 'white',
                                        fontSize: '12px',
                                        fontWeight: 600,
                                        flexShrink: 0
                                    }}>
                                        {msg.from.split(' ')[0][0]}{msg.from.split(' ')[1][0]}
                                    </div>

                                    {/* Message Info */}
                                    <div style={{ flex: '1' }}>
                                        <div style={{
                                            display: 'flex',
                                            gap: '6px',
                                            alignItems: 'center',
                                            marginBottom: '2px'
                                        }}>
                                            <span style={{
                                                fontSize: '14px',
                                                fontWeight: msg.unread ? 600 : 400,
                                                color: msg.unread ? '#2c3e50' : '#95a5a6'
                                            }}>
                                                {msg.from}
                                            </span>
                                            {msg.unread && (
                                                <span style={{
                                                    display: 'inline-block',
                                                    fontSize: '10px',
                                                    padding: '2px 4px',
                                                    backgroundColor: '#d32f2f',
                                                    color: 'white',
                                                    borderRadius: '2px',
                                                    fontWeight: 600
                                                }}>
                                                    NEW
                                                </span>
                                            )}
                                        </div>
                                        <p style={{
                                            fontSize: '12px',
                                            color: '#666',
                                            margin: '0',
                                            whiteSpace: 'nowrap',
                                            overflow: 'hidden',
                                            textOverflow: 'ellipsis'
                                        }}>
                                            {msg.text}
                                        </p>
                                    </div>

                                    {/* Time */}
                                    <span style={{
                                        fontSize: '12px',
                                        color: '#999',
                                        flexShrink: 0
                                    }}>
                                        {msg.time}
                                    </span>
                                </div>
                                {i < messages.length - 1 && (
                                    <div style={{
                                        height: '1px',
                                        backgroundColor: '#f0f0f0',
                                        margin: '0'
                                    }}></div>
                                )}
                            </div>
                        ))}
                    </div>
                </div>

                {/* Quick Reply Card */}
                <div style={{
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
                        Quick Reply
                    </h2>

                    <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
                        <div>
                            <label style={{
                                display: 'block',
                                fontSize: '12px',
                                fontWeight: 600,
                                color: '#666',
                                marginBottom: '4px'
                            }}>
                                To
                            </label>
                            <input
                                type="text"
                                placeholder="Recipient..."
                                style={{
                                    width: '100%',
                                    padding: '8px 12px',
                                    border: '1px solid #ddd',
                                    borderRadius: '4px',
                                    fontSize: '14px',
                                    fontFamily: 'inherit'
                                }}
                            />
                        </div>

                        <div>
                            <label style={{
                                display: 'block',
                                fontSize: '12px',
                                fontWeight: 600,
                                color: '#666',
                                marginBottom: '4px'
                            }}>
                                Message
                            </label>
                            <textarea
                                placeholder="Type your message..."
                                style={{
                                    width: '100%',
                                    padding: '8px 12px',
                                    border: '1px solid #ddd',
                                    borderRadius: '4px',
                                    fontSize: '14px',
                                    fontFamily: 'inherit',
                                    minHeight: '80px',
                                    resize: 'vertical'
                                }}
                            ></textarea>
                        </div>

                        <div style={{
                            display: 'flex',
                            gap: '8px',
                            justifyContent: 'flex-end'
                        }}>
                            <button style={{
                                padding: '8px 12px',
                                backgroundColor: 'white',
                                color: '#0066cc',
                                border: '1px solid #0066cc',
                                borderRadius: '4px',
                                fontSize: '12px',
                                fontWeight: 500,
                                cursor: 'pointer'
                            }}>
                                Cancel
                            </button>
                            <button style={{
                                padding: '8px 12px',
                                backgroundColor: '#0066cc',
                                color: 'white',
                                border: 'none',
                                borderRadius: '4px',
                                fontSize: '12px',
                                fontWeight: 500,
                                cursor: 'pointer'
                            }}>
                                Send
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};

render(<InboxDemo />, document.getElementById('root'));
