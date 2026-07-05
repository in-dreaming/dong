var count = 0;
var statusId = 0;
var __porf_auto_0Id = 0;
var incId = 0;

export function inc__onclick() {
  count = count + 1; setTextContent(statusId, String(count))
}

export function __porf_auto_0__onclick() {
  count = count + 1; setTextContent(statusId, String(count))
}

__porf_auto_0Id = getElementById('__porf_auto_0');
incId = getElementById('inc');
statusId = getElementById('status');
addEventListener(incId, 'click', 'inc__onclick');
addEventListener(__porf_auto_0Id, 'click', '__porf_auto_0__onclick');
