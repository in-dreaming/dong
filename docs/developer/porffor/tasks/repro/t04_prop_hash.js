let s = 0;

const o = { prop: 10, foo: 42 };
if (o.prop !== 10) s += 1;
if (o['prop'] !== 10) s += 2;
if (o.foo !== 42) s += 4;
if (o['foo'] !== 42) s += 8;

class C {
  field = 99;
  other = 7;
}
const c = new C();
if (c.field !== 99) s += 16;
if (c['field'] !== 99) s += 32;
if (c.other !== 7) s += 64;
if (c['other'] !== 7) s += 128;

const m = { f(x) { return x + 1; } };
if (m.f(1) !== 2) s += 256;

s;
