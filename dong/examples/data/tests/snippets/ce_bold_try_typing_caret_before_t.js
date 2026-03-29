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

    var sel = window.getSelection ? window.getSelection() : null;
    if (!sel || typeof document.createRange !== 'function') return;

    // Select "Try typing" then bold.
    var r = document.createRange();
    r.setStart(t, start);
    r.setEnd(t, start + needle.length);
    sel.removeAllRanges();
    sel.addRange(r);
    document.execCommand('bold');

    // Place caret before the 't' in "typing" (after "Try ").
    var textNode = null;
    function findText(n) {
        if (!n) return;
        if (n.nodeType === 3 && n.nodeValue && n.nodeValue.indexOf("Try typing") >= 0) {
            textNode = n;
            return;
        }
        var cs = n.childNodes || [];
        for (var i = 0; i < cs.length; i++) {
            findText(cs[i]);
            if (textNode) return;
        }
    }
    findText(e);
    if (!textNode) return;

    var s2 = textNode.nodeValue.indexOf("Try typing");
    if (s2 < 0) return;
    var off = s2 + 4; // before 't' in "typing"
    var r2 = document.createRange();
    r2.setStart(textNode, off);
    r2.collapse(true);
    sel.removeAllRanges();
    sel.addRange(r2);
})();
