import React, { Component, createRef } from 'react';
import { render } from '../../src/dong-react-renderer';
import '../../src/polyfills';

// Class component: single todo item
class TodoItem extends Component {
    constructor(props) {
        super(props);
        this.handleToggle = this.handleToggle.bind(this);
        this.handleDelete = this.handleDelete.bind(this);
    }

    shouldComponentUpdate(nextProps) {
        return nextProps.todo !== this.props.todo;
    }

    handleToggle() {
        this.props.onToggle(this.props.todo.id);
    }

    handleDelete() {
        this.props.onDelete(this.props.todo.id);
    }

    render() {
        const { todo } = this.props;
        return (
            <div style={{
                display: 'flex',
                alignItems: 'center',
                padding: '12px 16px',
                backgroundColor: '#fff',
                borderRadius: '8px',
                marginBottom: '8px',
                boxShadow: '0 1px 3px rgba(0,0,0,0.1)',
            }}>
                <div
                    onClick={this.handleToggle}
                    style={{
                        width: '24px',
                        height: '24px',
                        borderRadius: '50%',
                        border: todo.done ? 'none' : '2px solid #bdc3c7',
                        backgroundColor: todo.done ? '#27ae60' : 'transparent',
                        marginRight: '12px',
                        cursor: 'pointer',
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                        flexShrink: '0',
                    }}
                >
                    {todo.done && (
                        <span style={{ color: '#fff', fontSize: '14px', fontWeight: 'bold' }}>
                            V
                        </span>
                    )}
                </div>
                <span style={{
                    flex: '1',
                    fontSize: '16px',
                    color: todo.done ? '#95a5a6' : '#2c3e50',
                    textDecoration: todo.done ? 'line-through' : 'none',
                }}>
                    {todo.text}
                </span>
                <button
                    onClick={this.handleDelete}
                    style={{
                        backgroundColor: 'transparent',
                        border: 'none',
                        color: '#e74c3c',
                        fontSize: '18px',
                        cursor: 'pointer',
                        padding: '4px 8px',
                        borderRadius: '4px',
                    }}
                >
                    X
                </button>
            </div>
        );
    }
}

// Class component: filter bar
class FilterBar extends Component {
    render() {
        const { filter, onFilterChange, counts } = this.props;
        const filters = [
            { key: 'all', label: 'All (' + counts.total + ')' },
            { key: 'active', label: 'Active (' + counts.active + ')' },
            { key: 'done', label: 'Done (' + counts.done + ')' },
        ];

        return (
            <div style={{
                display: 'flex',
                gap: '8px',
                marginBottom: '16px',
            }}>
                {filters.map(f => (
                    <button
                        key={f.key}
                        onClick={() => onFilterChange(f.key)}
                        style={{
                            padding: '8px 16px',
                            backgroundColor: filter === f.key ? '#3498db' : '#ecf0f1',
                            color: filter === f.key ? '#fff' : '#7f8c8d',
                            border: 'none',
                            borderRadius: '6px',
                            fontSize: '13px',
                            cursor: 'pointer',
                            fontWeight: filter === f.key ? 'bold' : 'normal',
                        }}
                    >
                        {f.label}
                    </button>
                ))}
            </div>
        );
    }
}

// Main app: class component with full lifecycle
class TodoApp extends Component {
    constructor(props) {
        super(props);
        this.state = {
            todos: [
                { id: 1, text: 'Learn React class components', done: true },
                { id: 2, text: 'Build a custom React renderer', done: true },
                { id: 3, text: 'Integrate React with Dong engine', done: false },
                { id: 4, text: 'Ship the game UI', done: false },
            ],
            filter: 'all',
            inputText: '',
            nextId: 5,
        };
        this.inputRef = createRef();
        this.handleAdd = this.handleAdd.bind(this);
        this.handleToggle = this.handleToggle.bind(this);
        this.handleDelete = this.handleDelete.bind(this);
        this.handleFilterChange = this.handleFilterChange.bind(this);
        this.handleClearDone = this.handleClearDone.bind(this);
        this.handleInputChange = this.handleInputChange.bind(this);
        this.handleKeyDown = this.handleKeyDown.bind(this);
    }

    componentDidMount() {
        console.log('[TodoApp] mounted, ' + this.state.todos.length + ' items');
    }

    componentDidUpdate(prevProps, prevState) {
        if (prevState.todos.length !== this.state.todos.length) {
            console.log('[TodoApp] todo count changed: ' + this.state.todos.length);
        }
    }

    handleInputChange(e) {
        this.setState({ inputText: e.target.value });
    }

    handleKeyDown(e) {
        if (e.key === 'Enter') {
            this.handleAdd();
        }
    }

    handleAdd() {
        const text = this.state.inputText.trim();
        if (!text) return;
        this.setState(prev => ({
            todos: [...prev.todos, { id: prev.nextId, text, done: false }],
            nextId: prev.nextId + 1,
            inputText: '',
        }));
    }

    handleToggle(id) {
        this.setState(prev => ({
            todos: prev.todos.map(t =>
                t.id === id ? { ...t, done: !t.done } : t
            ),
        }));
    }

    handleDelete(id) {
        this.setState(prev => ({
            todos: prev.todos.filter(t => t.id !== id),
        }));
    }

    handleFilterChange(filter) {
        this.setState({ filter });
    }

    handleClearDone() {
        this.setState(prev => ({
            todos: prev.todos.filter(t => !t.done),
        }));
    }

    getFilteredTodos() {
        const { todos, filter } = this.state;
        if (filter === 'active') return todos.filter(t => !t.done);
        if (filter === 'done') return todos.filter(t => t.done);
        return todos;
    }

    render() {
        const filtered = this.getFilteredTodos();
        const { todos, filter, inputText } = this.state;
        const counts = {
            total: todos.length,
            active: todos.filter(t => !t.done).length,
            done: todos.filter(t => t.done).length,
        };

        return (
            <div style={{
                width: '100%',
                height: '100%',
                display: 'flex',
                alignItems: 'flex-start',
                justifyContent: 'center',
                backgroundColor: '#f0f3f5',
                fontFamily: 'sans-serif',
                paddingTop: '40px',
            }}>
                <div style={{
                    width: '480px',
                    padding: '24px',
                }}>
                    <h1 style={{
                        fontSize: '28px',
                        color: '#2c3e50',
                        marginBottom: '24px',
                        textAlign: 'center',
                    }}>
                        React Class Todo
                    </h1>

                    {/* Input area */}
                    <div style={{
                        display: 'flex',
                        gap: '8px',
                        marginBottom: '20px',
                    }}>
                        <input
                            ref={this.inputRef}
                            value={inputText}
                            onInput={this.handleInputChange}
                            onKeyDown={this.handleKeyDown}
                            placeholder="Add a new task..."
                            style={{
                                flex: '1',
                                padding: '12px 16px',
                                fontSize: '15px',
                                border: '2px solid #dcdde1',
                                borderRadius: '8px',
                                backgroundColor: '#fff',
                            }}
                        />
                        <button
                            onClick={this.handleAdd}
                            style={{
                                padding: '12px 20px',
                                backgroundColor: '#3498db',
                                color: '#fff',
                                border: 'none',
                                borderRadius: '8px',
                                fontSize: '15px',
                                cursor: 'pointer',
                                fontWeight: 'bold',
                            }}
                        >
                            Add
                        </button>
                    </div>

                    {/* Filter bar */}
                    <FilterBar
                        filter={filter}
                        onFilterChange={this.handleFilterChange}
                        counts={counts}
                    />

                    {/* Todo list */}
                    <div>
                        {filtered.map(todo => (
                            <TodoItem
                                key={todo.id}
                                todo={todo}
                                onToggle={this.handleToggle}
                                onDelete={this.handleDelete}
                            />
                        ))}
                    </div>

                    {/* Footer */}
                    {counts.done > 0 && (
                        <div style={{
                            marginTop: '16px',
                            textAlign: 'center',
                        }}>
                            <button
                                onClick={this.handleClearDone}
                                style={{
                                    padding: '8px 16px',
                                    backgroundColor: '#e74c3c',
                                    color: '#fff',
                                    border: 'none',
                                    borderRadius: '6px',
                                    fontSize: '13px',
                                    cursor: 'pointer',
                                }}
                            >
                                Clear done ({counts.done})
                            </button>
                        </div>
                    )}
                </div>
            </div>
        );
    }
}

render(<TodoApp />, document.getElementById('root'));
