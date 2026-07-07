var resultId = getElementById('result');

export function onFetchDone() {
  var ok = fetchOk();
  var status = fetchStatus();
  var body = fetchBody();
  if (ok === 0) {
    setTextContent(resultId, 'FAIL: ' + fetchError());
    classAdd(resultId, 'fail');
    return;
  }
  var expect = '{"greeting": "hello from dong", "version": 1, "items": ["alpha", "beta", "gamma"]}';
  if (status === 200 && body === expect) {
    setTextContent(resultId, 'PASS: greeting=hello from dong, items=alpha,beta,gamma');
    classAdd(resultId, 'pass');
  } else {
    setTextContent(resultId, 'FAIL: unexpected response');
    classAdd(resultId, 'fail');
  }
}

dongFetch('test_fetch_local.json', 'onFetchDone');
