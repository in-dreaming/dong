var focusHiddenElement = 0;
var setTabIndex = 0;
var testReadTabIndex = 0;
var __porf_auto_0Id = 0;
var __porf_auto_1Id = 0;
var __porf_auto_2Id = 0;
var __porf_auto_3Id = 0;
var __porf_auto_4Id = 0;

export function __porf_auto_0__onclick() {
  testReadTabIndex()
}

export function __porf_auto_1__onclick() {
  setTabIndex(0)
}

export function __porf_auto_2__onclick() {
  setTabIndex(1)
}

export function __porf_auto_3__onclick() {
  setTabIndex(-1)
}

export function __porf_auto_4__onclick() {
  focusHiddenElement()
}

__porf_auto_0Id = getElementById('__porf_auto_0');
__porf_auto_1Id = getElementById('__porf_auto_1');
__porf_auto_2Id = getElementById('__porf_auto_2');
__porf_auto_3Id = getElementById('__porf_auto_3');
__porf_auto_4Id = getElementById('__porf_auto_4');
addEventListener(__porf_auto_0Id, 'click', '__porf_auto_0__onclick');
addEventListener(__porf_auto_1Id, 'click', '__porf_auto_1__onclick');
addEventListener(__porf_auto_2Id, 'click', '__porf_auto_2__onclick');
addEventListener(__porf_auto_3Id, 'click', '__porf_auto_3__onclick');
addEventListener(__porf_auto_4Id, 'click', '__porf_auto_4__onclick');
