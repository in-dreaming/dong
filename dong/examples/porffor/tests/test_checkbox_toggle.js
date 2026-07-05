var resultId = getElementById('result');
var cb1Id = getElementById('cb1');
var cb2Id = getElementById('cb2');
var log = '';

if (getChecked(cb1Id) === 0) {
  log = log + 'cb1.checked initial: PASS\n';
} else {
  log = log + 'cb1.checked initial: FAIL\n';
}

if (getChecked(cb2Id) === 1) {
  log = log + 'cb2.checked initial: PASS\n';
} else {
  log = log + 'cb2.checked initial: FAIL\n';
}

setChecked(cb1Id, 1);
if (getChecked(cb1Id) === 1) {
  log = log + 'cb1.checked after JS set: PASS\n';
} else {
  log = log + 'cb1.checked after JS set: FAIL\n';
}

setChecked(cb1Id, 0);
if (getChecked(cb1Id) === 0) {
  log = log + 'cb1.checked after JS unset: PASS\n';
} else {
  log = log + 'cb1.checked after JS unset: FAIL\n';
}

setTextContent(resultId, log);
dongLog('test_checkbox_toggle porffor loaded');
