var inputId = getElementById('testInput');
var resultId = getElementById('result');
var v = getValue(inputId);
var msg = '';
if (v === 'Hello') {
  msg = 'initial value: PASS\n';
} else {
  msg = 'initial value: FAIL got=' + v + '\n';
}
setValue(inputId, 'Hello World');
var v2 = getValue(inputId);
if (v2 === 'Hello World') {
  msg = msg + 'setValue: PASS\n';
} else {
  msg = msg + 'setValue: FAIL got=' + v2 + '\n';
}
setTextContent(resultId, msg);
dongLog('test_input_value porffor loaded');
