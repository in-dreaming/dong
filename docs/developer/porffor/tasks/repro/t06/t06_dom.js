function check(label, got, want) {
  if (got !== want) {
    dongLog("FAIL " + label + " got=" + got + " want=" + want);
    return 0;
  }
  return 1;
}

function testValueRoundtrip() {
  setValue(10, "hello");
  return check("value", getValue(10), "hello");
}

function testCheckedDisabled() {
  setChecked(11, 1);
  setDisabled(11, 1);
  var ok = check("checked", getChecked(11), 1);
  ok = check("disabled", getDisabled(11), 1) && ok;
  setChecked(11, 0);
  setDisabled(11, 0);
  ok = check("unchecked", getChecked(11), 0) && ok;
  return check("enabled", getDisabled(11), 0) && ok;
}

function testAttribute() {
  setAttribute(12, "data-x", "帧率");
  var ok = check("attr", getAttribute(12, "data-x"), "帧率");
  removeAttribute(12, "data-x");
  return check("attr-rm", getAttribute(12, "data-x"), "") && ok;
}

function testQuery() {
  var id = querySelector(1, "#box");
  var ok = check("qsel", id, 42);
  ok = check("qsel-all", querySelectorAll(1, ".item"), "[7,8]") && ok;
  return check("by-tag", getElementsByTagName(1, "div"), "[7,8,42]") && ok;
}

function testClassList() {
  classAdd(20, "a");
  classAdd(20, "b");
  var ok = check("contains", classContains(20, "a"), 1);
  classRemove(20, "a");
  ok = check("removed", classContains(20, "a"), 0) && ok;
  var added = classToggle(20, "c");
  return check("toggle-add", added, 1) && ok;
}

function testStyle() {
  setStyle(30, "color", "red");
  var ok = check("style-get", getStyle(30, "color"), "red");
  return check("computed", getComputedStyleProp(30, "display"), "block") && ok;
}

function testInnerHTML() {
  setInnerHTML(13, "<span>ok</span>");
  return 1;
}

function testScrollLeft() {
  setScrollLeft(41, 5);
  return check("scroll-left", getScrollLeft(41), 0);
}

function testGeometry() {
  var ok = check("metric", getMetric(40, METRIC_OFFSET_WIDTH), 100);
  ok = check("scroll", getScrollTop(40), 12) && ok;
  setScrollTop(40, 24);
  ok = check("scroll-set", getScrollTop(40), 24) && ok;
  return check("rect", getRect(40), '{"x":1,"y":2,"w":100,"h":50}') && ok;
}

function testFocusMethods() {
  focusNode(50);
  blurNode(50);
  clickNode(50);
  var ok = check("matches", matchesSelector(50, ".btn"), 1);
  return check("closest", closestSelector(51, ".wrap"), 50) && ok;
}

function main() {
  var ok = 1;
  ok = testValueRoundtrip() && ok;
  ok = testCheckedDisabled() && ok;
  ok = testAttribute() && ok;
  ok = testQuery() && ok;
  ok = testClassList() && ok;
  ok = testStyle() && ok;
  ok = testInnerHTML() && ok;
  ok = testGeometry() && ok;
  ok = testScrollLeft() && ok;
  ok = testFocusMethods() && ok;
  if (ok) {
    dongLog("T06_PASS");
  } else {
    dongLog("T06_FAIL");
  }
}

main();
