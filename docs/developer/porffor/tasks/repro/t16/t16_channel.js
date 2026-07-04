// T16 string channel roundtrip — compiled with prelude + t16_stubs.c
var guardNode = 1;

function check(label, got, want) {
  if (got !== want) {
    dongLog("FAIL " + label + " got=" + got + " want=" + want);
    return 0;
  }
  return 1;
}

function testPullAscii() {
  dong_host_set_slot(toUtf8("hello"));
  return check("ascii", pullHostString(), "hello");
}

function testPullEmpty() {
  dong_host_set_slot(toUtf8(""));
  return check("empty", pullHostString(), "");
}

function testPullChinese() {
  dong_host_set_slot(toUtf8("帧率"));
  return check("zh", pullHostString(), "帧率");
}

function testPullEmoji() {
  dong_host_set_slot(toUtf8("hi🎮"));
  var got = pullHostString();
  return check("emoji", got, "hi🎮");
}

function testToUtf8Roundtrip() {
  var samples = "A中🎮";
  dong_host_set_slot(toUtf8(samples));
  return check("utf8-rt", pullHostString(), samples);
}

function testLong4k() {
  var chunk = "abcdefghij";
  var long = chunk + chunk + chunk + chunk;
  long = long + long;
  long = long + long;
  long = long + long;
  dong_host_set_slot(toUtf8(long));
  return check("4k-len", dong_str_len(), long.length);
}

function testFuzzOnce(seed) {
  var s = "f" + seed + "中" + seed;
  dong_host_set_slot(toUtf8(s));
  return check("fuzz" + seed, pullHostString(), s);
}

export function runOnce(seed) {
  return testFuzzOnce(seed);
}

function main() {
  var ok = 1;
  ok = testPullAscii() && ok;
  ok = testPullEmpty() && ok;
  ok = testPullChinese() && ok;
  ok = testPullEmoji() && ok;
  ok = testToUtf8Roundtrip() && ok;
  ok = testLong4k() && ok;
  var i = 0;
  while (i < 1000) {
    ok = testFuzzOnce(i) && ok;
    i = i + 1;
  }
  if (ok) {
    dongLog("T16_PASS");
  } else {
    dongLog("T16_FAIL");
  }
}
