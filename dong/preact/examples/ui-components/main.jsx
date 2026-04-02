import { Component, render } from 'preact/compat';
import '../../src/polyfills';
import { Button, Card, Badge, Input, Stack, Text, Avatar, Divider } from '../../src/ui';

class UIComponentsDemo extends Component {
    constructor(props) {
        super(props);
        this.state = {
            clickCount: 0,
            inputValue: '',
            selectedTab: 'buttons',
            messages: [
                { id: 1, from: 'Alice Chen', text: 'Hey, the new build looks great!', time: '2m ago', unread: true },
                { id: 2, from: 'Bob Wang', text: 'Can you review the PR?', time: '15m ago', unread: true },
                { id: 3, from: 'Carol Li', text: 'Meeting at 3pm tomorrow', time: '1h ago', unread: false },
                { id: 4, from: 'Dave Zhang', text: 'Shipped v2.0 to staging', time: '3h ago', unread: false },
            ],
        };
    }

    renderButtonsTab() {
        const { clickCount } = this.state;
        return (
            <Stack gap="16px">
                <Card title="Button Variants">
                    <Stack direction="row" gap="8px" style={{ flexWrap: 'wrap' }}>
                        <Button variant="primary" onClick={() => this.setState(s => ({ clickCount: s.clickCount + 1 }))}>
                            Primary ({clickCount})
                        </Button>
                        <Button variant="secondary">Secondary</Button>
                        <Button variant="danger">Danger</Button>
                        <Button variant="success">Success</Button>
                        <Button variant="ghost">Ghost</Button>
                    </Stack>
                </Card>

                <Card title="Button Sizes">
                    <Stack direction="row" gap="8px" align="center">
                        <Button size="sm">Small</Button>
                        <Button size="md">Medium</Button>
                        <Button size="lg">Large</Button>
                    </Stack>
                </Card>

                <Card title="Disabled & Block">
                    <Stack gap="8px">
                        <Stack direction="row" gap="8px">
                            <Button disabled>Disabled</Button>
                            <Button variant="danger" disabled>Disabled Danger</Button>
                        </Stack>
                        <Button variant="success" block>Block Button (full width)</Button>
                    </Stack>
                </Card>
            </Stack>
        );
    }

    renderCardsTab() {
        return (
            <Stack gap="16px">
                <Card title="User Profile">
                    <Stack direction="row" gap="12px" align="center">
                        <Avatar name="John Doe" size="lg" />
                        <Stack gap="2px">
                            <Text variant="h3">John Doe</Text>
                            <Text variant="caption">Senior Developer</Text>
                            <Stack direction="row" gap="4px" style={{ marginTop: '4px' }}>
                                <Badge color="blue">Preact</Badge>
                                <Badge color="green">C++</Badge>
                                <Badge color="yellow">GPU</Badge>
                            </Stack>
                        </Stack>
                    </Stack>
                </Card>

                <Card title="Project Stats">
                    <Stack gap="8px">
                        {[
                            { label: 'Commits', value: '1,247', badge: 'green' },
                            { label: 'Open Issues', value: '23', badge: 'yellow' },
                            { label: 'Contributors', value: '8', badge: 'blue' },
                        ].map(item => (
                            <Stack key={item.label} direction="row" justify="space-between" align="center">
                                <Text variant="body">{item.label}</Text>
                                <Badge color={item.badge}>{item.value}</Badge>
                            </Stack>
                        ))}
                    </Stack>
                </Card>

                <Card>
                    <Text variant="caption" align="center">
                        Card without title — just content
                    </Text>
                </Card>
            </Stack>
        );
    }

    renderInboxTab() {
        const { messages, inputValue } = this.state;
        const unreadCount = messages.filter(m => m.unread).length;

        return (
            <Stack gap="16px">
                <Card title={`Inbox ${unreadCount > 0 ? '(' + unreadCount + ' unread)' : ''}`}>
                    <Input
                        placeholder="Search messages..."
                        value={inputValue}
                        onInput={(e) => this.setState({ inputValue: e.target.value })}
                    />
                    <Stack gap="0px">
                        {messages
                            .filter(m => !inputValue || m.from.toLowerCase().includes(inputValue.toLowerCase())
                                || m.text.toLowerCase().includes(inputValue.toLowerCase()))
                            .map((msg, i, arr) => (
                                <div key={msg.id}>
                                    <Stack direction="row" gap="10px" align="center"
                                           style={{ padding: '10px 0', cursor: 'pointer' }}
                                           onClick={() => {
                                               this.setState(s => ({
                                                   messages: s.messages.map(m =>
                                                       m.id === msg.id ? { ...m, unread: !m.unread } : m
                                                   )
                                               }));
                                           }}>
                                        <Avatar name={msg.from} size="sm" />
                                        <Stack gap="2px" style={{ flex: '1' }}>
                                            <Stack direction="row" gap="6px" align="center">
                                                <Text variant="label" style={{
                                                    color: msg.unread ? '#2c3e50' : '#95a5a6',
                                                }}>
                                                    {msg.from}
                                                </Text>
                                                {msg.unread && <Badge color="red">NEW</Badge>}
                                            </Stack>
                                            <Text variant="caption">{msg.text}</Text>
                                        </Stack>
                                        <Text variant="caption">{msg.time}</Text>
                                    </Stack>
                                    {i < arr.length - 1 && <Divider style={{ margin: '0' }} />}
                                </div>
                            ))}
                    </Stack>
                </Card>

                <Card title="Quick Reply">
                    <Input label="To" placeholder="Recipient..." />
                    <Input label="Message" placeholder="Type your message..." />
                    <Stack direction="row" gap="8px" justify="flex-end">
                        <Button variant="secondary" size="sm">Cancel</Button>
                        <Button variant="primary" size="sm">Send</Button>
                    </Stack>
                </Card>
            </Stack>
        );
    }

    render() {
        const { selectedTab } = this.state;
        const tabs = [
            { key: 'buttons', label: 'Buttons' },
            { key: 'cards', label: 'Cards' },
            { key: 'inbox', label: 'Inbox' },
        ];

        return (
            <div style={{
                width: '100%',
                height: '100%',
                backgroundColor: '#f0f3f5',
                fontFamily: 'sans-serif',
                display: 'flex',
                justifyContent: 'center',
                paddingTop: '20px',
            }}>
                <div style={{ width: '520px', padding: '0 16px' }}>
                    <Text variant="h1" align="center" style={{ marginBottom: '4px' }}>
                        Dong UI Components
                    </Text>
                    <Text variant="caption" align="center" style={{ display: 'block', marginBottom: '20px' }}>
                        A lightweight Preact component library for Dong engine
                    </Text>

                    {/* Tab bar */}
                    <Stack direction="row" gap="4px" style={{ marginBottom: '16px' }}>
                        {tabs.map(tab => (
                            <Button
                                key={tab.key}
                                variant={selectedTab === tab.key ? 'primary' : 'secondary'}
                                size="sm"
                                onClick={() => this.setState({ selectedTab: tab.key })}
                                style={{ flex: '1' }}
                            >
                                {tab.label}
                            </Button>
                        ))}
                    </Stack>

                    {/* Content */}
                    {selectedTab === 'buttons' && this.renderButtonsTab()}
                    {selectedTab === 'cards' && this.renderCardsTab()}
                    {selectedTab === 'inbox' && this.renderInboxTab()}
                </div>
            </div>
        );
    }
}

render(<UIComponentsDemo />, document.getElementById('root'));
