// Repro path closer to manual: focus editor + select contents, then focus first Bold button
// (like clicking the button), then execCommand('bold'). Requires engine fix: JS focus on
// contenteditable sets last_editable_root_ (same as mousedown on CE).
(function () {
    var e = document.getElementById('editor');
    if (!e) return;
    e.focus();
    if (typeof document.createRange === 'function' && window.getSelection) {
        var r = document.createRange();
        r.selectNodeContents(e);
        var s = window.getSelection();
        if (s) {
            s.removeAllRanges();
            s.addRange(r);
        }
    } else {
        document.execCommand('selectAll');
    }
    var btn = document.getElementsByTagName('button')[0];
    if (btn) btn.focus();
    document.execCommand('bold');
})();
