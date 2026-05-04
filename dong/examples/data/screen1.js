// Screen 1 External JavaScript
console.log("[JS] Screen 1 external script loaded");

var clicks = 0;

function updateClickCount() {
    var countEl = document.getElementById('click-count');
    if (countEl) {
        countEl.textContent = 'Clicks: ' + clicks;
    }
}

function setStatus(msg) {
    var status = document.getElementById('status');
    if (status) {
        status.textContent = msg;
    }
}

// Button click handlers
var btnPrimary = document.getElementById('btn-primary');
var btnSecondary = document.getElementById('btn-secondary');
var btnReset = document.getElementById('btn-reset');

if (btnPrimary) {
    btnPrimary.addEventListener('click', function() {
        clicks++;
        updateClickCount();
        setStatus('Primary button clicked! Total: ' + clicks);
        console.log('[JS] Primary clicked, count:', clicks);
    });
}

if (btnSecondary) {
    btnSecondary.addEventListener('click', function() {
        clicks++;
        updateClickCount();
        setStatus('Secondary button clicked! Total: ' + clicks);
        console.log('[JS] Secondary clicked, count:', clicks);
    });
}

if (btnReset) {
    btnReset.addEventListener('click', function() {
        clicks = 0;
        updateClickCount();
        setStatus('Counter reset!');
        console.log('[JS] Reset clicked');
    });
}

// Input handlers
var nameInput = document.getElementById('name-input');
var messageInput = document.getElementById('message-input');

function updatePreview() {
    var name = nameInput ? nameInput.value : '(no name)';
    var msg = messageInput ? messageInput.value : '(no message)';
    var preview = document.getElementById('input-preview');
    if (preview) {
        preview.textContent = 'Preview: ' + (name || '(no name)') + ' says: ' + (msg || '(no message)');
    }
}

if (nameInput) {
    nameInput.addEventListener('input', function(e) {
        console.log('[JS] Name input changed: ' + e.target.value);
        updatePreview();
    });
}

if (messageInput) {
    messageInput.addEventListener('input', function(e) {
        console.log('[JS] Message input changed: ' + e.target.value);
        updatePreview();
    });
}

console.log("[JS] Screen 1 event handlers installed");
