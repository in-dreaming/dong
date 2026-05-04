(function () {
    var e = document.getElementById('editor');
    if (!e) return;
    e.focus();

    var t = null;
    for (var i = 0; i < e.childNodes.length; i++) {
        if (e.childNodes[i] && e.childNodes[i].nodeType === 3) {
            t = e.childNodes[i];
            break;
        }
    }
    if (!t || !t.nodeValue) return;

    var src = t.nodeValue;
    var needle = "Try typing";
    var start = src.indexOf(needle);
    if (start < 0) return;

    if (typeof document.createRange === 'function' && window.getSelection) {
        var r = document.createRange();
        r.setStart(t, start);
        r.setEnd(t, start + needle.length);
        var s = window.getSelection();
        if (s) {
            s.removeAllRanges();
            s.addRange(r);
        }
    }

    document.execCommand('underline');
})();
