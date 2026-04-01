import Reconciler from 'react-reconciler';
import { DefaultEventPriority } from 'react-reconciler/constants';

const EVENT_MAP = {
    onClick:       'click',
    onMouseDown:   'mousedown',
    onMouseUp:     'mouseup',
    onMouseMove:   'mousemove',
    onMouseEnter:  'mouseenter',
    onMouseLeave:  'mouseleave',
    onKeyDown:     'keydown',
    onKeyUp:       'keyup',
    onInput:       'input',
    onChange:       'change',
    onFocus:       'focus',
    onBlur:        'blur',
    onScroll:      'scroll',
};

function applyProp(node, key, value) {
    if (key === 'children') {
        if (typeof value === 'string' || typeof value === 'number') {
            node.textContent = String(value);
        }
        return;
    }
    if (key === 'style' && typeof value === 'object') {
        for (const [k, v] of Object.entries(value || {})) {
            const cssProp = k.replace(/[A-Z]/g, m => '-' + m.toLowerCase());
            node.style.setProperty(cssProp, String(v));
        }
        return;
    }
    if (key === 'className') {
        node.setAttribute('class', value || '');
        return;
    }
    if (key.startsWith('on') && EVENT_MAP[key]) {
        const eventName = EVENT_MAP[key];
        if (!node.__listeners) node.__listeners = {};
        if (node.__listeners[key]) {
            node.removeEventListener(eventName, node.__listeners[key]);
        }
        if (typeof value === 'function') {
            node.__listeners[key] = value;
            node.addEventListener(eventName, value);
        } else {
            delete node.__listeners[key];
        }
        return;
    }
    if (value === null || value === undefined || value === false) {
        node.removeAttribute(key);
    } else if (value === true) {
        node.setAttribute(key, '');
    } else {
        node.setAttribute(key, String(value));
    }
}

function removeProp(node, key) {
    if (key === 'children' || key === 'style') return;
    if (key.startsWith('on') && EVENT_MAP[key]) {
        const eventName = EVENT_MAP[key];
        if (node.__listeners && node.__listeners[key]) {
            node.removeEventListener(eventName, node.__listeners[key]);
            delete node.__listeners[key];
        }
        return;
    }
    node.removeAttribute(key === 'className' ? 'class' : key);
}

const hostConfig = {
    supportsMutation: true,
    supportsPersistence: false,
    supportsHydration: false,
    isPrimaryRenderer: true,
    noTimeout: -1,

    createInstance(type, props) {
        const el = document.createElement(type);
        for (const [key, value] of Object.entries(props)) {
            applyProp(el, key, value);
        }
        return el;
    },

    createTextInstance(text) {
        return document.createTextNode(text);
    },

    appendInitialChild(parent, child) { parent.appendChild(child); },
    appendChild(parent, child) { parent.appendChild(child); },
    appendChildToContainer(container, child) { container.appendChild(child); },

    removeChild(parent, child) { parent.removeChild(child); },
    removeChildFromContainer(container, child) { container.removeChild(child); },

    insertBefore(parent, child, before) { parent.insertBefore(child, before); },
    insertInContainerBefore(container, child, before) {
        container.insertBefore(child, before);
    },

    prepareUpdate(node, type, oldProps, newProps) {
        const payload = [];
        const allKeys = new Set([
            ...Object.keys(oldProps),
            ...Object.keys(newProps),
        ]);
        for (const key of allKeys) {
            if (key === 'children') {
                const o = oldProps.children, n = newProps.children;
                if ((typeof o === 'string' || typeof o === 'number' ||
                     typeof n === 'string' || typeof n === 'number') && o !== n) {
                    payload.push(key);
                }
                continue;
            }
            if (oldProps[key] !== newProps[key]) {
                payload.push(key);
            }
        }
        return payload.length > 0 ? payload : null;
    },

    commitUpdate(node, payload, type, oldProps, newProps) {
        for (const key of payload) {
            if (!(key in newProps) || newProps[key] === undefined) {
                removeProp(node, key);
            } else {
                applyProp(node, key, newProps[key]);
            }
        }
    },

    commitTextUpdate(node, oldText, newText) {
        node.textContent = newText;
    },

    resetTextContent(node) { node.textContent = ''; },

    shouldSetTextContent(type, props) {
        return typeof props.children === 'string'
            || typeof props.children === 'number';
    },

    clearContainer(container) { container.innerHTML = ''; },

    getRootHostContext() { return null; },
    getChildHostContext(parentCtx) { return parentCtx; },
    getPublicInstance(instance) { return instance; },
    prepareForCommit() { return null; },
    resetAfterCommit() {},

    finalizeInitialChildren(node, type, props) {
        if (props.autoFocus) {
            setTimeout(() => node.focus(), 0);
            return true;
        }
        return false;
    },

    scheduleTimeout: setTimeout,
    cancelTimeout: clearTimeout,
    getCurrentEventPriority() { return DefaultEventPriority; },

    beforeActiveInstanceBlur() {},
    afterActiveInstanceBlur() {},
    prepareScopeUpdate() {},
    getInstanceFromScope() { return null; },
    detachDeletedInstance() {},
};

const reconciler = Reconciler(hostConfig);

export function render(element, container) {
    let root = container.__reactRoot;
    if (!root) {
        root = reconciler.createContainer(
            container,
            0,
            null,
            false,
            null,
            '',
            (err) => console.error(err),
            null,
        );
        container.__reactRoot = root;
    }
    reconciler.updateContainer(element, root, null, null);
    return root;
}

export function unmount(container) {
    if (container.__reactRoot) {
        reconciler.updateContainer(null, container.__reactRoot, null, null);
        delete container.__reactRoot;
    }
}
