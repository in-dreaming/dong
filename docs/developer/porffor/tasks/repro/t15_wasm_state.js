let count = 0;

export function onClick() {
  count = count + 1;
}

export function getCount() {
  return count;
}

export function echoArg(x) {
  return x;
}
