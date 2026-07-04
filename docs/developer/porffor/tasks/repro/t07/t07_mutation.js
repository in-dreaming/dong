function check(label, got, want) {
  if (got !== want) {
    dongLog("FAIL " + label + " got=" + got + " want=" + want);
    return 0;
  }
  return 1;
}

function testCreateAppend() {
  var parent = 1;
  var child = createElement("div");
  appendChild(parent, child);
  setTextContent(child, "hi");
  var ok = check("append-parent", getParentId(child), parent);
  ok = check("append-text", getTextContent(child), "hi") && ok;
  return ok;
}

function testCreateTextNode() {
  var parent = 1;
  var tn = createTextNode("txt");
  appendChild(parent, tn);
  return check("text-node", getTextContent(tn), "txt");
}

function testInsertBeforeAppend() {
  var parent = 1;
  var a = createElement("div");
  appendChild(parent, a);
  var b = createElement("span");
  insertBefore(parent, b, 0);
  var ok = check("ins0-parent", getParentId(b), parent);
  return check("ins0-sibling", getNextSiblingId(a), 0) && ok;
}

function testInsertBeforeRef() {
  var parent = 1;
  while (getFirstChildId(parent) !== 0) {
    removeNode(getFirstChildId(parent));
  }
  var a = createElement("div");
  var c = createElement("div");
  appendChild(parent, a);
  appendChild(parent, c);
  var b = createElement("div");
  insertBefore(parent, b, c);
  var ok = check("ins-ref-order", getNextSiblingId(a), b);
  ok = check("ins-ref-next", getNextSiblingId(b), c) && ok;
  return ok;
}

function testRemove() {
  var parent = 1;
  var child = createElement("div");
  appendChild(parent, child);
  var id = child;
  removeNode(child);
  var ok = check("removed-parent", getParentId(id), 0);
  appendChild(parent, id);
  return check("stale-noop", getParentId(id), 0) && ok;
}

function testReplaceChild() {
  var parent = 1;
  while (getFirstChildId(parent) !== 0) {
    removeNode(getFirstChildId(parent));
  }
  var old = createElement("div");
  appendChild(parent, old);
  var neu = createElement("span");
  replaceChild(parent, neu, old);
  var ok = check("replace-first", getFirstChildId(parent), neu);
  ok = check("replace-old-stale", getParentId(old), 0) && ok;
  return ok;
}

function testTraversal() {
  var parent = 1;
  while (getFirstChildId(parent) !== 0) {
    removeNode(getFirstChildId(parent));
  }
  var a = createElement("div");
  var b = createElement("div");
  appendChild(parent, a);
  appendChild(parent, b);
  var ok = check("first", getFirstChildId(parent), a);
  ok = check("next", getNextSiblingId(a), b) && ok;
  return check("parent", getParentId(b), parent) && ok;
}

function testCloneNode() {
  var parent = 1;
  var src = createElement("div");
  appendChild(parent, src);
  setTextContent(src, "x");
  var clone = cloneNodeId(src, 0);
  var ok = check("clone-id", clone > 0, 1);
  ok = check("clone-detached", getParentId(clone), 0) && ok;
  appendChild(parent, clone);
  return check("clone-text", getTextContent(clone), "x") && ok;
}

function testInvalidIds() {
  appendChild(0, 1);
  appendChild(1, 0);
  insertBefore(1, 2, 0);
  removeNode(0);
  removeNode(99999);
  replaceChild(1, 2, 3);
  var ok = check("invalid-parent", getParentId(0), 0);
  ok = check("invalid-first", getFirstChildId(0), 0) && ok;
  return check("invalid-sibling", getNextSiblingId(99999), 0) && ok;
}

function testStress() {
  var parent = 1;
  while (getFirstChildId(parent) !== 0) {
    removeNode(getFirstChildId(parent));
  }
  var baseline = dongStateGetNum(0);
  var ids = [];
  var i = 0;
  while (i < 500) {
    var n = createElement("div");
    appendChild(parent, n);
    ids[i] = n;
    i = i + 1;
  }
  var mid = dongStateGetNum(0);
  i = 0;
  while (i < 500) {
    removeNode(ids[i]);
    i = i + 1;
  }
  var end = dongStateGetNum(0);
  var ok = check("stress-mid", mid, baseline + 500);
  ok = check("stress-end", end, baseline) && ok;
  return ok;
}

function main() {
  dongStateSetNum(0, 1);
  var ok = 1;
  ok = testCreateAppend() && ok;
  ok = testCreateTextNode() && ok;
  ok = testInsertBeforeAppend() && ok;
  ok = testInsertBeforeRef() && ok;
  ok = testRemove() && ok;
  ok = testReplaceChild() && ok;
  ok = testTraversal() && ok;
  ok = testCloneNode() && ok;
  ok = testInvalidIds() && ok;
  ok = testStress() && ok;
  if (ok) {
    dongLog("T07_PASS");
  } else {
    dongLog("T07_FAIL");
  }
}

main();
