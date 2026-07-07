var clearLog = 0;
var logClick = 0;
var toggleDisabled = 0;
var __porf_auto_0Id = 0;
var __porf_auto_1Id = 0;
var __porf_auto_2Id = 0;
var __porf_auto_3Id = 0;
var __porf_auto_4Id = 0;
var __porf_auto_5Id = 0;
var __porf_auto_6Id = 0;
var __porf_auto_7Id = 0;
var __porf_auto_8Id = 0;
var button1Id = 0;
var submitBtnId = 0;

export function __porf_auto_0__onclick() {
  toggleDisabled('textInput')
}

export function __porf_auto_1__onclick() {
  toggleDisabled('checkbox1')
}

export function __porf_auto_2__onclick() {
  toggleDisabled('radio1')
}

export function __porf_auto_3__onclick() {
  toggleDisabled('radio2')
}

export function __porf_auto_4__onclick() {
  toggleDisabled('select1')
}

export function __porf_auto_5__onclick() {
  toggleDisabled('textarea1')
}

export function button1__onclick() {
  logClick('button1')
}

export function __porf_auto_6__onclick() {
  toggleDisabled('button1')
}

export function submitBtn__onclick() {
  logClick('submitBtn')
}

export function __porf_auto_7__onclick() {
  toggleDisabled('submitBtn')
}

export function __porf_auto_8__onclick() {
  clearLog()
}

__porf_auto_0Id = getElementById('__porf_auto_0');
__porf_auto_1Id = getElementById('__porf_auto_1');
__porf_auto_2Id = getElementById('__porf_auto_2');
__porf_auto_3Id = getElementById('__porf_auto_3');
__porf_auto_4Id = getElementById('__porf_auto_4');
__porf_auto_5Id = getElementById('__porf_auto_5');
__porf_auto_6Id = getElementById('__porf_auto_6');
__porf_auto_7Id = getElementById('__porf_auto_7');
__porf_auto_8Id = getElementById('__porf_auto_8');
button1Id = getElementById('button1');
submitBtnId = getElementById('submitBtn');
addEventListener(__porf_auto_0Id, 'click', '__porf_auto_0__onclick');
addEventListener(__porf_auto_1Id, 'click', '__porf_auto_1__onclick');
addEventListener(__porf_auto_2Id, 'click', '__porf_auto_2__onclick');
addEventListener(__porf_auto_3Id, 'click', '__porf_auto_3__onclick');
addEventListener(__porf_auto_4Id, 'click', '__porf_auto_4__onclick');
addEventListener(__porf_auto_5Id, 'click', '__porf_auto_5__onclick');
addEventListener(button1Id, 'click', 'button1__onclick');
addEventListener(__porf_auto_6Id, 'click', '__porf_auto_6__onclick');
addEventListener(submitBtnId, 'click', 'submitBtn__onclick');
addEventListener(__porf_auto_7Id, 'click', '__porf_auto_7__onclick');
addEventListener(__porf_auto_8Id, 'click', '__porf_auto_8__onclick');
