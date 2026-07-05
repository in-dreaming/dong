var resultId = getElementById('result');
var h1Id = getElementById('h1');
var toggleId = getElementById('toggle');
var r = '';

var h1Hidden = getAttribute(h1Id, 'hidden');
if (h1Hidden === '') {
  r = r + 'h1 has hidden attr: PASS\n';
} else {
  r = r + 'h1 has hidden attr: FAIL\n';
}

setAttribute(toggleId, 'hidden', '');
var tHidden = getAttribute(toggleId, 'hidden');
if (tHidden === '') {
  r = r + 'toggle.hidden set to true: PASS\n';
} else {
  r = r + 'toggle.hidden set to true: FAIL\n';
}

removeAttribute(toggleId, 'hidden');
var tShown = getAttribute(toggleId, 'hidden');
if (tShown === '') {
  r = r + 'toggle.hidden set to false: FAIL\n';
} else {
  r = r + 'toggle.hidden set to false: PASS\n';
}

setTextContent(resultId, r);
dongLog('test_hidden_attr porffor loaded');
