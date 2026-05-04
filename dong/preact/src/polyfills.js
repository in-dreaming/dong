// Polyfills for Preact scheduler in QuickJS/Dong environment.
// requestAnimationFrame is natively implemented in Dong (js_bindings.cpp),
// these polyfills are fallbacks in case the native version is unavailable.

if (typeof globalThis.requestAnimationFrame === 'undefined') {
    globalThis.requestAnimationFrame = function(cb) {
        return setTimeout(function() { cb(performance.now()); }, 16);
    };
    globalThis.cancelAnimationFrame = function(id) {
        clearTimeout(id);
    };
}

if (typeof globalThis.queueMicrotask === 'undefined') {
    globalThis.queueMicrotask = function(fn) {
        Promise.resolve().then(fn);
    };
}
