var resultId = getElementById('result');

export function onFetchDone() {
  var ok = fetchOk();
  var body = fetchBody();
  if (ok === 0) {
    setTextContent(resultId, 'FAIL: ' + fetchError());
    classAdd(resultId, 'fail');
    return;
  }
  var expect = 'This is a local text file loaded via fetch().\nIt should appear in the DOM after the fetch completes.';
  if (body === expect) {
    setTextContent(resultId, 'PASS: local text file loaded');
    classAdd(resultId, 'pass');
  } else {
    setTextContent(resultId, 'FAIL: unexpected content');
    classAdd(resultId, 'fail');
  }
}

dongFetch('test_fetch_local.txt', 'onFetchDone');
