function logResult(name, ok) {
  console.log((ok ? 'PASS' : 'FAIL') + ':' + name);
}

function test_then_resolve() {
  Promise.resolve(42).then(function (v) {
    logResult('then_resolve', v === 42);
  });
}

function test_catch_reject() {
  Promise.reject('err').catch(function (e) {
    logResult('catch_reject', e === 'err');
  });
}

function test_finally() {
  Promise.resolve(1).finally(function () {
    logResult('finally', true);
  });
}

function test_chain() {
  Promise.resolve(1)
    .then(function (v) {
      return v + 1;
    })
    .then(function (v) {
      logResult('chain', v === 2);
    });
}

function test_all() {
  Promise.all([Promise.resolve(1), Promise.resolve(2)]).then(function (arr) {
    logResult('all', arr.length === 2 && arr[0] === 1 && arr[1] === 2);
  });
}

function test_race() {
  Promise.race([Promise.resolve('win'), new Promise(function () {})]).then(function (v) {
    logResult('race', v === 'win');
  });
}

function test_settled_sync() {
  Promise.resolve(99).then(function (v) {
    logResult('settled_sync', v === 99);
  });
}

function test_microtask_order() {
  var order = '';
  Promise.resolve().then(function () {
    order = order + 'm';
  });
  setTimeout(function () {
    order = order + 't';
    logResult('microtask_order', order === 'mt');
  }, 0);
}

function main() {
  test_then_resolve();
  test_catch_reject();
  test_finally();
  test_chain();
  test_all();
  test_race();
  test_settled_sync();
  test_microtask_order();
  return 0;
}
