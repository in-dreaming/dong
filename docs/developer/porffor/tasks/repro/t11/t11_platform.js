export function probeClipboardCn() {
  clipboardWrite("你好，Dong 剪贴板");
  var s = clipboardRead();
  dongLog("CLIP:" + s);
}

export function probeMatchMedia() {
  var below = matchMedia("(min-width: 900px)");
  dongLog("MM:" + (below ? "1" : "0"));
}

export function probeCssSupports() {
  var ok = cssSupports("display", "flex");
  var bad = cssSupports("display", "grid");
  dongLog("CSS:" + (ok ? "1" : "0") + "," + (bad ? "0" : "1"));
}

export function probeScene() {
  sceneClear();
  var id = sceneAddNode(
    '{"name":"box","x":10,"y":20,"w":100,"h":40,"background":"#336699","text":"Scene"}',
  );
  var found = sceneFind("box");
  dongLog("SCENE:" + String(id) + "," + String(found) + "," + String(sceneCount()));
}

export function probeTextLayout() {
  clearOverlay();
  var cfg =
    '{"text":"Hello Dong","font":{"family":"sans-serif","size":16},"columns":[{"x":0,"y":0,"width":200,"height":80}]}';
  var out = textLayout(cfg);
  dongLog("TL:" + (out.indexOf("Hello") >= 0 ? "1" : "0"));
}

function main() {
  dongLog("T11_INIT");
}
