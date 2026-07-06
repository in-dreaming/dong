var MAX_TODOS = 16;
var todoCount = 4;
var nextId = 5;
var filterMode = 0;
var inputText = '';

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

var todoInputId = 0;
var btnAddId = 0;
var filterAllId = 0;
var filterActiveId = 0;
var filterDoneId = 0;
var todoListId = 0;
var clearWrapId = 0;
var btnClearId = 0;

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
  return todoId15;
}

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
  return todoText15;
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
  return todoDone15;
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
  todoDone15 = v;
}

function setTodoSlot(i, id, text, done) {
  if (i === 0) { todoId0 = id; todoText0 = text; todoDone0 = done; return; }
  if (i === 1) { todoId1 = id; todoText1 = text; todoDone1 = done; return; }
  if (i === 2) { todoId2 = id; todoText2 = text; todoDone2 = done; return; }
  if (i === 3) { todoId3 = id; todoText3 = text; todoDone3 = done; return; }
  if (i === 4) { todoId4 = id; todoText4 = text; todoDone4 = done; return; }
  if (i === 5) { todoId5 = id; todoText5 = text; todoDone5 = done; return; }
  if (i === 6) { todoId6 = id; todoText6 = text; todoDone6 = done; return; }
  if (i === 7) { todoId7 = id; todoText7 = text; todoDone7 = done; return; }
  if (i === 8) { todoId8 = id; todoText8 = text; todoDone8 = done; return; }
  if (i === 9) { todoId9 = id; todoText9 = text; todoDone9 = done; return; }
  if (i === 10) { todoId10 = id; todoText10 = text; todoDone10 = done; return; }
  if (i === 11) { todoId11 = id; todoText11 = text; todoDone11 = done; return; }
  if (i === 12) { todoId12 = id; todoText12 = text; todoDone12 = done; return; }
  if (i === 13) { todoId13 = id; todoText13 = text; todoDone13 = done; return; }
  if (i === 14) { todoId14 = id; todoText14 = text; todoDone14 = done; return; }
  todoId15 = id;
  todoText15 = text;
  todoDone15 = done;
}

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

function countActive() {
  var n = 0;
  var d = 0;
  var i = 0;
  while (i < todoCount) {
    if (todoDoneAt(i)) {
      d = d + 1;
    } else {
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

function porfRebuildList() {
  var html = '';
  var i = 0;
  while (i < todoCount) {
    if (shouldShow(i)) {
      var tid = todoIdAt(i);
      var text = todoTextAt(i);
      var done = todoDoneAt(i);
      var strike = '';
      var color = '#2c3e50';
      var checkBg = 'transparent';
      var checkBorder = '2px solid #bdc3c7';
      var checkMark = '';
      if (done) {
        strike = 'line-through';
        color = '#95a5a6';
        checkBg = '#27ae60';
        checkBorder = 'none';
        checkMark = 'V';
      }
      html = html + '<div style="display:flex;align-items:center;padding:12px 16px;background:#fff;border-radius:8px;margin-bottom:8px;box-shadow:0 1px 3px rgba(0,0,0,0.1);">';
      html = html + '<div data-todo-toggle-index="' + String(i) + '" style="width:24px;height:24px;border-radius:50%;border:' + checkBorder + ';background:' + checkBg + ';margin-right:12px;display:flex;align-items:center;justify-content:center;color:#fff;font-size:14px;font-weight:bold;cursor:pointer;">' + checkMark + '</div>';
      html = html + '<span style="flex:1;font-size:16px;color:' + color + ';text-decoration:' + strike + ';">' + text + '</span>';
      html = html + '<button data-todo-delete-index="' + String(i) + '" style="background:transparent;border:none;color:#e74c3c;font-size:18px;padding:4px 8px;cursor:pointer;">X</button>';
      html = html + '</div>';
    }
    i = i + 1;
  }
  setInnerHTML(todoListId, html);
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
  setTextContent(filterAllId, 'All (' + String(total) + ')');
  setTextContent(filterActiveId, 'Active (' + String(active) + ')');
  setTextContent(filterDoneId, 'Done (' + String(done) + ')');
  var allBg = filterMode === 0 ? '#3498db' : '#ecf0f1';
  var allColor = filterMode === 0 ? '#fff' : '#7f8c8d';
  var actBg = filterMode === 1 ? '#3498db' : '#ecf0f1';
  var actColor = filterMode === 1 ? '#fff' : '#7f8c8d';
  var doneBg = filterMode === 2 ? '#3498db' : '#ecf0f1';
  var doneColor = filterMode === 2 ? '#fff' : '#7f8c8d';
  setStyle(filterAllId, 'background-color', allBg);
  setStyle(filterAllId, 'color', allColor);
  setStyle(filterActiveId, 'background-color', actBg);
  setStyle(filterActiveId, 'color', actColor);
  setStyle(filterDoneId, 'background-color', doneBg);
  setStyle(filterDoneId, 'color', doneColor);
  if (done > 0) {
    removeAttribute(clearWrapId, 'hidden');
    setTextContent(btnClearId, 'Clear done (' + String(done) + ')');
  } else {
    setAttribute(clearWrapId, 'hidden', '1');
  }
}

function porfRefresh() {
  porfPatchFilters();
  porfRebuildList();
}

function removeAtIndex(idx) {
  var j = idx;
  while (j + 1 < todoCount) {
    setTodoSlot(j, todoIdAt(j + 1), todoTextAt(j + 1), todoDoneAt(j + 1));
    j = j + 1;
  }
  todoCount = todoCount - 1;
}

export function onAdd() {
  var text = getValue(todoInputId);
  var trimmed = text;
  if (trimmed.length === 0) {
    return;
  }
  if (todoCount >= MAX_TODOS) {
    dongLog('todo max reached');
    return;
  }
  setTodoSlot(todoCount, nextId, trimmed, 0);
  todoCount = todoCount + 1;
  nextId = nextId + 1;
  setValue(todoInputId, '');
  porfRefresh();
}

export function onInputChange() {
  inputText = getValue(todoInputId);
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

todoInputId = getElementById('todo-input');
btnAddId = getElementById('btn-add');
filterAllId = getElementById('filter-all');
filterActiveId = getElementById('filter-active');
filterDoneId = getElementById('filter-done');
todoListId = getElementById('todo-list');
clearWrapId = getElementById('clear-wrap');
btnClearId = getElementById('btn-clear');

addEventListener(btnAddId, 'click', 'onAdd');
addEventListener(todoInputId, 'input', 'onInputChange');
addEventListener(todoInputId, 'keydown', 'onKeyDown');
addEventListener(filterAllId, 'click', 'onFilterAll');
addEventListener(filterActiveId, 'click', 'onFilterActive');
addEventListener(filterDoneId, 'click', 'onFilterDone');
addEventListener(btnClearId, 'click', 'onClearDone');
addEventListener(todoListId, 'click', 'onListClick');

porfRefresh();
dongLog('porf_todo loaded');
