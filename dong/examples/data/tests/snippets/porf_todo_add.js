(function () {
  var input = document.getElementById('todo-input');
  if (input) {
    input.value = 'New task from headless test';
  }
  var btn = document.getElementById('btn-add');
  if (btn) {
    btn.click();
  }
})();
