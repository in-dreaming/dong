var editor = getElementById('editor');
var logId = getElementById('log');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\n');
}

focusNode(editor);
appendLog('selectAll: ' + (selectAll() ? 'true' : 'false'));
appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
selectAll();
appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
selectAll();
appendLog('bold toggle: ' + (execCommand('bold', '') ? 'true' : 'false'));
