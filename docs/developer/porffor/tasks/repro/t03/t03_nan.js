let a = NaN;
let b = Infinity;
let c = -Infinity;
let ok = Number.isNaN(a) && b > 1e100 && c < -1e100;
ok ? 42 : 0;
