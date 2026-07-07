var testElementFromPoint = 0;
var testHasFocus = 0;
var testScrollingElement = 0;
var __porf_auto_0Id = 0;
var __porf_auto_1Id = 0;
var __porf_auto_2Id = 0;

export function __porf_auto_0__onclick() {
  testElementFromPoint()
}

export function __porf_auto_1__onclick() {
  testHasFocus()
}

export function __porf_auto_2__onclick() {
  testScrollingElement()
}

__porf_auto_0Id = getElementById('__porf_auto_0');
__porf_auto_1Id = getElementById('__porf_auto_1');
__porf_auto_2Id = getElementById('__porf_auto_2');
addEventListener(__porf_auto_0Id, 'click', '__porf_auto_0__onclick');
addEventListener(__porf_auto_1Id, 'click', '__porf_auto_1__onclick');
addEventListener(__porf_auto_2Id, 'click', '__porf_auto_2__onclick');
