var scrollAutoBottom = 0;
var scrollAutoTop = 0;
var scrollSmoothBottom = 0;
var scrollSmoothTop = 0;
var __porf_auto_0Id = 0;
var __porf_auto_1Id = 0;
var __porf_auto_2Id = 0;
var __porf_auto_3Id = 0;

export function __porf_auto_0__onclick() {
  scrollAutoTop()
}

export function __porf_auto_1__onclick() {
  scrollAutoBottom()
}

export function __porf_auto_2__onclick() {
  scrollSmoothTop()
}

export function __porf_auto_3__onclick() {
  scrollSmoothBottom()
}

__porf_auto_0Id = getElementById('__porf_auto_0');
__porf_auto_1Id = getElementById('__porf_auto_1');
__porf_auto_2Id = getElementById('__porf_auto_2');
__porf_auto_3Id = getElementById('__porf_auto_3');
addEventListener(__porf_auto_0Id, 'click', '__porf_auto_0__onclick');
addEventListener(__porf_auto_1Id, 'click', '__porf_auto_1__onclick');
addEventListener(__porf_auto_2Id, 'click', '__porf_auto_2__onclick');
addEventListener(__porf_auto_3Id, 'click', '__porf_auto_3__onclick');
