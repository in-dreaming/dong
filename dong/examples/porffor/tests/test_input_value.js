var inputId = getElementById('testInput');
var resultId = getElementById('result');
var v = getValue(inputId);
var log = 'Input value: ' + v + '\n';
setValue(inputId, 'Changed');
var v2 = getValue(inputId);
log = log + 'After set: ' + v2 + '\n';
setTextContent(resultId, log);
dongLog('test_input_value porffor loaded');
