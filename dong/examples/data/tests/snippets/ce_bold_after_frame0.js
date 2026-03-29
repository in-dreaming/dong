// Run after frame 0: focus #editor, select contents, bold (execCommand while focus on editor).
// If this does not reproduce a bug seen via dong_app (click Bold after selecting text), try
// snippets/ce_bold_like_button_click.js — same as clicking the toolbar button (focus on button).
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
    document.execCommand('bold');
})();
