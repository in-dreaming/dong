// hello_dom — Porffor smoke test: DOM text + click handler by export name.

var statusId = getElementById('status');
var btnId = getElementById('btn');
addEventListener(btnId, 'click', 'onBtnClick');
setTextContent(statusId, 'ready');
dongLog('hello_dom loaded');
