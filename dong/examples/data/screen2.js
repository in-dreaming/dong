// Screen 2 External JavaScript
console.log("[JS] Screen 2 external script loaded");

var clicks = 0;

var btnPrimary = document.getElementById('btn-primary');
var clickCount = document.getElementById('click-count');

if (btnPrimary) {
    btnPrimary.addEventListener('click', function() {
        clicks++;
        if (clickCount) {
            clickCount.textContent = 'Clicks: ' + clicks;
        }
        console.log('[JS] Screen 2 Hello clicked, count:', clicks);
    });
    console.log('[JS] Screen 2 btn-primary listener installed');
} else {
    console.log('[JS] Screen 2 btn-primary not found');
}

// Scroll handler
var scrollArea = document.getElementById('scroll-area');
if (scrollArea) {
    scrollArea.addEventListener('wheel', function(e) {
        console.log('[JS] Scrolling in screen 2');
    });
}

console.log("[JS] Screen 2 event handlers installed");
