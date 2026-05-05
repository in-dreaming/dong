/**
 * dong-ui TypeScript type definitions
 * @packageDocumentation
 */

// ============================================================================
// Core Engine Types
// ============================================================================

/** DOM Element (subset of standard HTMLElement API supported by dong) */
interface DongElement {
  tagName: string;
  id: string;
  className: string;
  textContent: string;
  innerHTML: string;

  // Attributes
  getAttribute(name: string): string | null;
  setAttribute(name: string, value: string): void;
  hasAttribute(name: string): boolean;
  removeAttribute(name: string): void;

  // DOM traversal
  parentElement: DongElement | null;
  children: DongElement[];
  firstElementChild: DongElement | null;
  lastElementChild: DongElement | null;
  previousElementSibling: DongElement | null;
  nextElementSibling: DongElement | null;
  childElementCount: number;

  // DOM manipulation
  appendChild(child: DongElement): DongElement;
  removeChild(child: DongElement): DongElement;
  insertBefore(newChild: DongElement, refChild: DongElement | null): DongElement;

  // Query
  querySelector(selector: string): DongElement | null;
  querySelectorAll(selector: string): DongElement[];
  matches(selector: string): boolean;
  closest(selector: string): DongElement | null;
  contains(other: DongElement): boolean;

  // Events
  addEventListener(type: string, handler: (event: any) => void): void;
  removeEventListener(type: string, handler: (event: any) => void): void;
  dispatchEvent(event: Event): boolean;

  // Style
  style: CSSStyleDeclaration;
  dataset: DOMStringMap;

  // Geometry
  getBoundingClientRect(): DOMRect;
  offsetTop: number;
  offsetLeft: number;
  offsetWidth: number;
  offsetHeight: number;
  clientWidth: number;
  clientHeight: number;
  scrollTop: number;
  scrollLeft: number;
  scrollWidth: number;
  scrollHeight: number;

  // Content editable
  isContentEditable: boolean;

  // Node info
  nodeType: number;
  isConnected: boolean;
}

/** Document interface */
interface DongDocument {
  body: DongElement;
  head: DongElement;
  documentElement: DongElement;

  getElementById(id: string): DongElement | null;
  getElementsByTagName(tag: string): DongElement[];
  getElementsByClassName(cls: string): DongElement[];
  querySelector(selector: string): DongElement | null;
  querySelectorAll(selector: string): DongElement[];

  createElement(tag: string): DongElement;
  createElementNS(ns: string, tag: string): DongElement;
  createTextNode(text: string): DongElement;
  createDocumentFragment(): DocumentFragment;
  createComment(text: string): Comment;

  addEventListener(type: string, handler: (event: any) => void): void;
  removeEventListener(type: string, handler: (event: any) => void): void;
}

/** Window interface */
interface DongWindow {
  document: DongDocument;
  innerWidth: number;
  innerHeight: number;

  // Timers
  setTimeout(fn: () => void, delay?: number): number;
  clearTimeout(id: number): void;
  setInterval(fn: () => void, delay?: number): number;
  clearInterval(id: number): void;
  requestAnimationFrame(fn: (timestamp: number) => void): number;
  cancelAnimationFrame(id: number): void;
  queueMicrotask(fn: () => void): void;

  // Storage
  localStorage: Storage;
  sessionStorage: Storage;

  // Performance
  performance: { now(): number };

  // Style
  getComputedStyle(element: DongElement): CSSStyleDeclaration;
  matchMedia(query: string): MediaQueryList;

  // Events
  addEventListener(type: string, handler: (event: any) => void): void;
  removeEventListener(type: string, handler: (event: any) => void): void;

  // Navigation
  location: { href: string; pathname: string; search: string; hash: string };
}

// ============================================================================
// dong-specific APIs
// ============================================================================

/** Spatial navigation direction */
type DongNavDirection = 'up' | 'down' | 'left' | 'right' | 'next' | 'prev';

/** dong global object */
interface Dong {
  /** Trigger spatial navigation in a direction */
  focusNav(direction: DongNavDirection): void;

  /** Get a named view (for multi-view scenarios) */
  getView(name: string): DongWindow | null;
}

declare const dong: Dong;
declare const document: DongDocument;
declare const window: DongWindow;

// ============================================================================
// Standard Web APIs available in dong
// ============================================================================

declare class Event {
  constructor(type: string, options?: { bubbles?: boolean; cancelable?: boolean });
  type: string;
  target: DongElement | null;
  currentTarget: DongElement | null;
  bubbles: boolean;
  cancelable: boolean;
  defaultPrevented: boolean;
  preventDefault(): void;
  stopPropagation(): void;
}

declare class CustomEvent extends Event {
  constructor(type: string, options?: { bubbles?: boolean; cancelable?: boolean; detail?: any });
  detail: any;
}

declare class MouseEvent extends Event {
  constructor(type: string, options?: any);
  clientX: number;
  clientY: number;
  button: number;
}

declare class KeyboardEvent extends Event {
  constructor(type: string, options?: any);
  key: string;
  code: string;
  keyCode: number;
  ctrlKey: boolean;
  shiftKey: boolean;
  altKey: boolean;
  metaKey: boolean;
}

declare class MessageChannel {
  port1: MessagePort;
  port2: MessagePort;
}

interface MessagePort {
  onmessage: ((event: { data: any }) => void) | null;
  postMessage(data: any): void;
}

declare class MutationObserver {
  constructor(callback: (mutations: any[], observer: MutationObserver) => void);
  observe(target: DongElement, options?: { childList?: boolean; subtree?: boolean; attributes?: boolean }): void;
  disconnect(): void;
}

declare class ResizeObserver {
  constructor(callback: (entries: any[]) => void);
  observe(target: DongElement): void;
  unobserve(target: DongElement): void;
  disconnect(): void;
}

declare class IntersectionObserver {
  constructor(callback: (entries: any[]) => void, options?: { threshold?: number | number[] });
  observe(target: DongElement): void;
  unobserve(target: DongElement): void;
  disconnect(): void;
}

// Fetch API
declare function fetch(url: string, options?: { method?: string; headers?: Record<string, string>; body?: string }): Promise<Response>;

interface Response {
  ok: boolean;
  status: number;
  text(): Promise<string>;
  json(): Promise<any>;
}

// Structured clone
declare function structuredClone<T>(value: T): T;
