let count = 0;
let statusId = 1;

function setTextContent(nodeId, text) {}

export function onClick() {
  count = count + 1;
  setTextContent(statusId, String(count));
}

export function echoArg(x) {
  return x;
}

setTextContent(statusId, '0');
