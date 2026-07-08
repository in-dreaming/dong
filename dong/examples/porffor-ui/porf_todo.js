var porf_rootId = 0;
var titleId = 0;
var todo_inputId = 0;
var btn_addId = 0;
var filter_barId = 0;
var filter_allId = 0;
var filter_activeId = 0;
var filter_doneId = 0;
var todo_listId = 0;
var clear_wrapId = 0;
var btn_clearId = 0;

function todoTextAt(i) {
  if (i === 0) return todoText0;
  if (i === 1) return todoText1;
  if (i === 2) return todoText2;
  if (i === 3) return todoText3;
  if (i === 4) return todoText4;
  if (i === 5) return todoText5;
  if (i === 6) return todoText6;
  if (i === 7) return todoText7;
  if (i === 8) return todoText8;
  if (i === 9) return todoText9;
  if (i === 10) return todoText10;
  if (i === 11) return todoText11;
  if (i === 12) return todoText12;
  if (i === 13) return todoText13;
  if (i === 14) return todoText14;
  if (i === 15) return todoText15;
  if (i === 16) return todoText16;
  if (i === 17) return todoText17;
  if (i === 18) return todoText18;
  if (i === 19) return todoText19;
  if (i === 20) return todoText20;
  if (i === 21) return todoText21;
  if (i === 22) return todoText22;
  if (i === 23) return todoText23;
  if (i === 24) return todoText24;
  if (i === 25) return todoText25;
  if (i === 26) return todoText26;
  if (i === 27) return todoText27;
  if (i === 28) return todoText28;
  if (i === 29) return todoText29;
  if (i === 30) return todoText30;
  if (i === 31) return todoText31;
  return todoText31;
}

function setTodoTextAt(i, v) {
  if (i === 0) { todoText0 = v; return; }
  if (i === 1) { todoText1 = v; return; }
  if (i === 2) { todoText2 = v; return; }
  if (i === 3) { todoText3 = v; return; }
  if (i === 4) { todoText4 = v; return; }
  if (i === 5) { todoText5 = v; return; }
  if (i === 6) { todoText6 = v; return; }
  if (i === 7) { todoText7 = v; return; }
  if (i === 8) { todoText8 = v; return; }
  if (i === 9) { todoText9 = v; return; }
  if (i === 10) { todoText10 = v; return; }
  if (i === 11) { todoText11 = v; return; }
  if (i === 12) { todoText12 = v; return; }
  if (i === 13) { todoText13 = v; return; }
  if (i === 14) { todoText14 = v; return; }
  if (i === 15) { todoText15 = v; return; }
  if (i === 16) { todoText16 = v; return; }
  if (i === 17) { todoText17 = v; return; }
  if (i === 18) { todoText18 = v; return; }
  if (i === 19) { todoText19 = v; return; }
  if (i === 20) { todoText20 = v; return; }
  if (i === 21) { todoText21 = v; return; }
  if (i === 22) { todoText22 = v; return; }
  if (i === 23) { todoText23 = v; return; }
  if (i === 24) { todoText24 = v; return; }
  if (i === 25) { todoText25 = v; return; }
  if (i === 26) { todoText26 = v; return; }
  if (i === 27) { todoText27 = v; return; }
  if (i === 28) { todoText28 = v; return; }
  if (i === 29) { todoText29 = v; return; }
  if (i === 30) { todoText30 = v; return; }
  if (i === 31) { todoText31 = v; return; }
  todoText31 = v;
}

function todoDoneAt(i) {
  if (i === 0) return todoDone0;
  if (i === 1) return todoDone1;
  if (i === 2) return todoDone2;
  if (i === 3) return todoDone3;
  if (i === 4) return todoDone4;
  if (i === 5) return todoDone5;
  if (i === 6) return todoDone6;
  if (i === 7) return todoDone7;
  if (i === 8) return todoDone8;
  if (i === 9) return todoDone9;
  if (i === 10) return todoDone10;
  if (i === 11) return todoDone11;
  if (i === 12) return todoDone12;
  if (i === 13) return todoDone13;
  if (i === 14) return todoDone14;
  if (i === 15) return todoDone15;
  if (i === 16) return todoDone16;
  if (i === 17) return todoDone17;
  if (i === 18) return todoDone18;
  if (i === 19) return todoDone19;
  if (i === 20) return todoDone20;
  if (i === 21) return todoDone21;
  if (i === 22) return todoDone22;
  if (i === 23) return todoDone23;
  if (i === 24) return todoDone24;
  if (i === 25) return todoDone25;
  if (i === 26) return todoDone26;
  if (i === 27) return todoDone27;
  if (i === 28) return todoDone28;
  if (i === 29) return todoDone29;
  if (i === 30) return todoDone30;
  if (i === 31) return todoDone31;
  return todoDone31;
}

function setTodoDoneAt(i, v) {
  if (i === 0) { todoDone0 = v; return; }
  if (i === 1) { todoDone1 = v; return; }
  if (i === 2) { todoDone2 = v; return; }
  if (i === 3) { todoDone3 = v; return; }
  if (i === 4) { todoDone4 = v; return; }
  if (i === 5) { todoDone5 = v; return; }
  if (i === 6) { todoDone6 = v; return; }
  if (i === 7) { todoDone7 = v; return; }
  if (i === 8) { todoDone8 = v; return; }
  if (i === 9) { todoDone9 = v; return; }
  if (i === 10) { todoDone10 = v; return; }
  if (i === 11) { todoDone11 = v; return; }
  if (i === 12) { todoDone12 = v; return; }
  if (i === 13) { todoDone13 = v; return; }
  if (i === 14) { todoDone14 = v; return; }
  if (i === 15) { todoDone15 = v; return; }
  if (i === 16) { todoDone16 = v; return; }
  if (i === 17) { todoDone17 = v; return; }
  if (i === 18) { todoDone18 = v; return; }
  if (i === 19) { todoDone19 = v; return; }
  if (i === 20) { todoDone20 = v; return; }
  if (i === 21) { todoDone21 = v; return; }
  if (i === 22) { todoDone22 = v; return; }
  if (i === 23) { todoDone23 = v; return; }
  if (i === 24) { todoDone24 = v; return; }
  if (i === 25) { todoDone25 = v; return; }
  if (i === 26) { todoDone26 = v; return; }
  if (i === 27) { todoDone27 = v; return; }
  if (i === 28) { todoDone28 = v; return; }
  if (i === 29) { todoDone29 = v; return; }
  if (i === 30) { todoDone30 = v; return; }
  if (i === 31) { todoDone31 = v; return; }
  todoDone31 = v;
}

function todoIdAt(i) {
  if (i === 0) return todoId0;
  if (i === 1) return todoId1;
  if (i === 2) return todoId2;
  if (i === 3) return todoId3;
  if (i === 4) return todoId4;
  if (i === 5) return todoId5;
  if (i === 6) return todoId6;
  if (i === 7) return todoId7;
  if (i === 8) return todoId8;
  if (i === 9) return todoId9;
  if (i === 10) return todoId10;
  if (i === 11) return todoId11;
  if (i === 12) return todoId12;
  if (i === 13) return todoId13;
  if (i === 14) return todoId14;
  if (i === 15) return todoId15;
  if (i === 16) return todoId16;
  if (i === 17) return todoId17;
  if (i === 18) return todoId18;
  if (i === 19) return todoId19;
  if (i === 20) return todoId20;
  if (i === 21) return todoId21;
  if (i === 22) return todoId22;
  if (i === 23) return todoId23;
  if (i === 24) return todoId24;
  if (i === 25) return todoId25;
  if (i === 26) return todoId26;
  if (i === 27) return todoId27;
  if (i === 28) return todoId28;
  if (i === 29) return todoId29;
  if (i === 30) return todoId30;
  if (i === 31) return todoId31;
  return todoId31;
}

function porfRebuild_todos() {
  var html = '<div>';
  var i = 0;
  while (i < todoCount) {
    if (i >= 32) {
      dongLog('porfRebuild_todos: MAX_ITEMS 32 truncated');
      break;
    }
    if (shouldShow(i)) {
    html = html + buildTodoRow(i);
    }
    i = i + 1;
  }
  html = html + '</div>';
  setInnerHTML(todo_listId, html);
}

function setTodoSlot(i, text, done, id) {
  if (i === 0) { todoText0 = text; todoDone0 = done; todoId0 = id; return; }
  if (i === 1) { todoText1 = text; todoDone1 = done; todoId1 = id; return; }
  if (i === 2) { todoText2 = text; todoDone2 = done; todoId2 = id; return; }
  if (i === 3) { todoText3 = text; todoDone3 = done; todoId3 = id; return; }
  if (i === 4) { todoText4 = text; todoDone4 = done; todoId4 = id; return; }
  if (i === 5) { todoText5 = text; todoDone5 = done; todoId5 = id; return; }
  if (i === 6) { todoText6 = text; todoDone6 = done; todoId6 = id; return; }
  if (i === 7) { todoText7 = text; todoDone7 = done; todoId7 = id; return; }
  if (i === 8) { todoText8 = text; todoDone8 = done; todoId8 = id; return; }
  if (i === 9) { todoText9 = text; todoDone9 = done; todoId9 = id; return; }
  if (i === 10) { todoText10 = text; todoDone10 = done; todoId10 = id; return; }
  if (i === 11) { todoText11 = text; todoDone11 = done; todoId11 = id; return; }
  if (i === 12) { todoText12 = text; todoDone12 = done; todoId12 = id; return; }
  if (i === 13) { todoText13 = text; todoDone13 = done; todoId13 = id; return; }
  if (i === 14) { todoText14 = text; todoDone14 = done; todoId14 = id; return; }
  if (i === 15) { todoText15 = text; todoDone15 = done; todoId15 = id; return; }
  if (i === 16) { todoText16 = text; todoDone16 = done; todoId16 = id; return; }
  if (i === 17) { todoText17 = text; todoDone17 = done; todoId17 = id; return; }
  if (i === 18) { todoText18 = text; todoDone18 = done; todoId18 = id; return; }
  if (i === 19) { todoText19 = text; todoDone19 = done; todoId19 = id; return; }
  if (i === 20) { todoText20 = text; todoDone20 = done; todoId20 = id; return; }
  if (i === 21) { todoText21 = text; todoDone21 = done; todoId21 = id; return; }
  if (i === 22) { todoText22 = text; todoDone22 = done; todoId22 = id; return; }
  if (i === 23) { todoText23 = text; todoDone23 = done; todoId23 = id; return; }
  if (i === 24) { todoText24 = text; todoDone24 = done; todoId24 = id; return; }
  if (i === 25) { todoText25 = text; todoDone25 = done; todoId25 = id; return; }
  if (i === 26) { todoText26 = text; todoDone26 = done; todoId26 = id; return; }
  if (i === 27) { todoText27 = text; todoDone27 = done; todoId27 = id; return; }
  if (i === 28) { todoText28 = text; todoDone28 = done; todoId28 = id; return; }
  if (i === 29) { todoText29 = text; todoDone29 = done; todoId29 = id; return; }
  if (i === 30) { todoText30 = text; todoDone30 = done; todoId30 = id; return; }
  if (i === 31) { todoText31 = text; todoDone31 = done; todoId31 = id; return; }
  todoText31 = text; todoDone31 = done; todoId31 = id;
}

function porfPatchIf_clear_wrap() {
  if (showClear) {
    removeAttribute(clear_wrapId, 'hidden');
  } else {
    setAttribute(clear_wrapId, 'hidden', '1');
  }
}

function porfInit() {
  porf_rootId = getElementById('porf-root');
  titleId = getElementById('title');
  todo_inputId = getElementById('todo-input');
  btn_addId = getElementById('btn-add');
  filter_barId = getElementById('filter-bar');
  filter_allId = getElementById('filter-all');
  filter_activeId = getElementById('filter-active');
  filter_doneId = getElementById('filter-done');
  todo_listId = getElementById('todo-list');
  clear_wrapId = getElementById('clear-wrap');
  btn_clearId = getElementById('btn-clear');
  addEventListener(todo_inputId, 'input', 'onInputChange');
  addEventListener(todo_inputId, 'keydown', 'onKeyDown');
  addEventListener(btn_addId, 'click', 'onAdd');
  addEventListener(filter_allId, 'click', 'onFilterAll');
  addEventListener(filter_activeId, 'click', 'onFilterActive');
  addEventListener(filter_doneId, 'click', 'onFilterDone');
  addEventListener(todo_listId, 'click', 'onListClick');
  addEventListener(btn_clearId, 'click', 'onClearDone');
  porfPatchIf_clear_wrap();
  porfRebuild_todos();
  porfRefresh();
  dongLog('porfInit');
}

var MAX_TODOS = 16;
var todoCount = 4;
var nextId = 5;
var filterMode = 0;
var inputText = '';
var showClear = 0;

var todoId0 = 1;
var todoText0 = 'Learn Porffor class components';
var todoDone0 = 1;
var todoId1 = 2;
var todoText1 = 'Compare bundle size with React';
var todoDone1 = 1;
var todoId2 = 3;
var todoText2 = 'Integrate Porffor with Dong engine';
var todoDone2 = 0;
var todoId3 = 4;
var todoText3 = 'Ship the game UI';
var todoDone3 = 0;

var todoId4 = 0;
var todoText4 = '';
var todoDone4 = 0;
var todoId5 = 0;
var todoText5 = '';
var todoDone5 = 0;
var todoId6 = 0;
var todoText6 = '';
var todoDone6 = 0;
var todoId7 = 0;
var todoText7 = '';
var todoDone7 = 0;
var todoId8 = 0;
var todoText8 = '';
var todoDone8 = 0;
var todoId9 = 0;
var todoText9 = '';
var todoDone9 = 0;
var todoId10 = 0;
var todoText10 = '';
var todoDone10 = 0;
var todoId11 = 0;
var todoText11 = '';
var todoDone11 = 0;
var todoId12 = 0;
var todoText12 = '';
var todoDone12 = 0;
var todoId13 = 0;
var todoText13 = '';
var todoDone13 = 0;
var todoId14 = 0;
var todoText14 = '';
var todoDone14 = 0;
var todoId15 = 0;
var todoText15 = '';
var todoDone15 = 0;

function normalizeInitialTodoText() {
  todoText0 = persistStr(todoText0);
  todoText1 = persistStr(todoText1);
  todoText2 = persistStr(todoText2);
  todoText3 = persistStr(todoText3);
}

function countActive() {
  var n = 0;
  var i = 0;
  while (i < todoCount) {
    if (!todoDoneAt(i)) {
      n = n + 1;
    }
    i = i + 1;
  }
  return n;
}

function countDone() {
  var d = 0;
  var i = 0;
  while (i < todoCount) {
    if (todoDoneAt(i)) {
      d = d + 1;
    }
    i = i + 1;
  }
  return d;
}

function shouldShow(i) {
  var done = todoDoneAt(i);
  if (filterMode === 1) {
    if (done) return 0;
    return 1;
  }
  if (filterMode === 2) {
    if (done) return 1;
    return 0;
  }
  return 1;
}

function buildTodoRow(i) {
  var text = todoTextAt(i);
  var done = todoDoneAt(i);
  var doneClass = '';
  var checkMark = '';
  if (done) {
    doneClass = ' done';
    checkMark = 'V';
  }
  var html = '';
  html = html + '<div class="todo-row' + doneClass + '">';
  html = html + '<div class="todo-check" data-todo-toggle-index="' + numToStr(i) + '">' + checkMark + '</div>';
  html = html + '<span class="todo-text">' + text + '</span>';
  html = html + '<button class="todo-delete" data-todo-delete-index="' + numToStr(i) + '">X</button>';
  html = html + '</div>';
  return html;
}

function parseIndexStr(s) {
  if (s === '0') return 0;
  if (s === '1') return 1;
  if (s === '2') return 2;
  if (s === '3') return 3;
  if (s === '4') return 4;
  if (s === '5') return 5;
  if (s === '6') return 6;
  if (s === '7') return 7;
  if (s === '8') return 8;
  if (s === '9') return 9;
  if (s === '10') return 10;
  if (s === '11') return 11;
  if (s === '12') return 12;
  if (s === '13') return 13;
  if (s === '14') return 14;
  if (s === '15') return 15;
  return -1;
}

function readToggleIndex() {
  var target = eventTarget();
  if (target === 0) {
    return -1;
  }
  var v = getAttribute(target, 'data-todo-toggle-index');
  if (v.length > 0) {
    return parseIndexStr(v);
  }
  var hit = closestSelector(target, '[data-todo-toggle-index]');
  if (hit !== 0) {
    v = getAttribute(hit, 'data-todo-toggle-index');
    return parseIndexStr(v);
  }
  return -1;
}

function readDeleteIndex() {
  var target = eventTarget();
  if (target === 0) {
    return -1;
  }
  var v = getAttribute(target, 'data-todo-delete-index');
  if (v.length > 0) {
    return parseIndexStr(v);
  }
  var hit = closestSelector(target, '[data-todo-delete-index]');
  if (hit !== 0) {
    v = getAttribute(hit, 'data-todo-delete-index');
    return parseIndexStr(v);
  }
  return -1;
}

function porfPatchFilters() {
  var total = todoCount;
  var active = countActive();
  var done = countDone();
  setTextContent(filter_allId, 'All (' + String(total) + ')');
  setTextContent(filter_activeId, 'Active (' + String(active) + ')');
  setTextContent(filter_doneId, 'Done (' + String(done) + ')');
  var allBg = filterMode === 0 ? '#3498db' : '#ecf0f1';
  var allColor = filterMode === 0 ? '#fff' : '#7f8c8d';
  var actBg = filterMode === 1 ? '#3498db' : '#ecf0f1';
  var actColor = filterMode === 1 ? '#fff' : '#7f8c8d';
  var doneBg = filterMode === 2 ? '#3498db' : '#ecf0f1';
  var doneColor = filterMode === 2 ? '#fff' : '#7f8c8d';
  setStyle(filter_allId, 'background-color', allBg);
  setStyle(filter_allId, 'color', allColor);
  setStyle(filter_activeId, 'background-color', actBg);
  setStyle(filter_activeId, 'color', actColor);
  setStyle(filter_doneId, 'background-color', doneBg);
  setStyle(filter_doneId, 'color', doneColor);
  if (done > 0) {
    showClear = 1;
    setTextContent(btn_clearId, 'Clear done (' + String(done) + ')');
  } else {
    showClear = 0;
  }
  porfPatchIf_clear_wrap();
}

function porfRefresh() {
  porfPatchFilters();
  porfRebuild_todos();
}

function removeAtIndex(idx) {
  var j = idx;
  while (j + 1 < todoCount) {
    setTodoSlot(j, todoTextAt(j + 1), todoDoneAt(j + 1), todoIdAt(j + 1));
    j = j + 1;
  }
  todoCount = todoCount - 1;
}

export function onAdd() {
  var text = persistStr(getValue(todo_inputId));
  if (text.length === 0) {
    return;
  }
  if (todoCount >= MAX_TODOS) {
    dongLog('todo max reached');
    return;
  }
  setTodoSlot(todoCount, text, 0, nextId);
  todoCount = todoCount + 1;
  nextId = nextId + 1;
  setValue(todo_inputId, '');
  porfRefresh();
}

export function onInputChange() {
  inputText = persistStr(getValue(todo_inputId));
}

export function headlessAddSample() {
  setValue(todo_inputId, 'Porffor smoke task');
  onAdd();
}

export function onKeyDown() {
  var key = eventKey();
  if (key === 'Enter') {
    onAdd();
  }
}

export function onFilterAll() {
  filterMode = 0;
  porfRefresh();
}

export function onFilterActive() {
  filterMode = 1;
  porfRefresh();
}

export function onFilterDone() {
  filterMode = 2;
  porfRefresh();
}

export function onClearDone() {
  var i = todoCount - 1;
  while (i >= 0) {
    if (todoDoneAt(i)) {
      removeAtIndex(i);
    }
    i = i - 1;
  }
  porfRefresh();
}

export function onListClick() {
  var idx = readToggleIndex();
  if (idx >= 0 && idx < todoCount) {
    var d = todoDoneAt(idx);
    if (d) {
      setTodoDoneAt(idx, 0);
    } else {
      setTodoDoneAt(idx, 1);
    }
    porfRefresh();
    return;
  }
  var idx2 = readDeleteIndex();
  if (idx2 >= 0 && idx2 < todoCount) {
    removeAtIndex(idx2);
    porfRefresh();
  }
}

normalizeInitialTodoText();

porfInit();
