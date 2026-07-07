var resultId = getElementById('result');

export function onFetchDone() {
  var ok = fetchOk();
  var err = fetchError();
  if (ok === 0 && err !== '') {
    setTextContent(resultId, 'PASS: fetch correctly rejected: ' + err);
    classAdd(resultId, 'pass');
  } else {
    setTextContent(resultId, 'FAIL: expected rejection');
    classAdd(resultId, 'fail');
  }
}

dongFetch('nonexistent_file_xyz.txt', 'onFetchDone');
