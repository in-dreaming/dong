var editor = getElementById('editor');
var logId = getElementById('log');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\n');
}

focusNode(editor);
selectAll();
appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('insertText: ' + (execCommand('insertText', 'Replaced') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('delete: ' + (execCommand('delete', '') ? 'true' : 'false'));
