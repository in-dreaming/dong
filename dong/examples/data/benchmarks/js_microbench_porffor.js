// js_microbench_porffor.js — Porffor-compatible microbench (globals only, no cross-scope closures).

var bench_name = '';
var bench_iterations = 0;
var bench_start = 0;

function bench_finish() {
  var elapsed = dong_time_now() - bench_start;
  var nsPerIter = (elapsed * 1000000 / bench_iterations).toFixed(1);
  var line = '[BENCH] ' + bench_name + ': ' + elapsed + 'ms (' + bench_iterations + ' iterations, ' + nsPerIter + ' ns/iter)';
  benchLog(line);
}

// 1. property_access
bench_name = 'property_access';
bench_iterations = 1000000;
bench_start = dong_time_now();
var i = 0;
var obj_x = 0;
while (i < bench_iterations) {
  var obj = { x: 1, y: 2, z: 3 };
  obj_x = obj.x + obj.y + obj.z;
  i = i + 1;
}
bench_finish();

// 2. function_call
bench_name = 'function_call';
bench_iterations = 200000;
bench_start = dong_time_now();
i = 0;
while (i < bench_iterations) {
  noop(); noop(); noop(); noop(); noop();
  i = i + 1;
}
bench_finish();

function noop() {}

// 3. object_create
bench_name = 'object_create';
bench_iterations = 100000;
bench_start = dong_time_now();
i = 0;
while (i < bench_iterations) {
  var o = { a: 1, b: 2, c: 3 };
  i = i + 1;
}
bench_finish();

// 4. json_parse (small fixed string)
bench_name = 'json_parse';
bench_iterations = 1000;
var jsonStr = '{"items":[{"id":1,"name":"item1"}]}';
bench_start = dong_time_now();
i = 0;
while (i < bench_iterations) {
  var parsed = JSON.parse(jsonStr);
  i = i + 1;
}
bench_finish();

// 5. string_concat
bench_name = 'string_concat';
bench_iterations = 10000;
bench_start = dong_time_now();
i = 0;
while (i < bench_iterations) {
  var s = '';
  var j = 0;
  while (j < 100) {
    s = s + 'x';
    j = j + 1;
  }
  i = i + 1;
}
bench_finish();

benchLog('[BENCH] DONE');
dongLog('[BENCH] DONE');
