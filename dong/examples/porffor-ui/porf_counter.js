var porf_rootId = 0;
var titleId = 0;
var count_displayId = 0;
var btn_decId = 0;
var btn_resetId = 0;
var btn_incId = 0;

function porfPatch_count() {
  setTextContent(count_displayId, String(count));
}

function porfPatchStyle_countColor_color() {
  setStyle(count_displayId, 'color', String(countColor));
}

function porfInit() {
  porf_rootId = getElementById('porf-root');
  titleId = getElementById('title');
  count_displayId = getElementById('count-display');
  btn_decId = getElementById('btn-dec');
  btn_resetId = getElementById('btn-reset');
  btn_incId = getElementById('btn-inc');
  addEventListener(btn_decId, 'click', 'onDec');
  addEventListener(btn_resetId, 'click', 'onReset');
  addEventListener(btn_incId, 'click', 'onInc');
  porfPatch_count();
  porfPatchStyle_countColor_color();
  dongLog('porfInit');
}

var count = 0;
var countColor = '#27ae60';

function porfSyncCountColor() {
  if (count >= 0) {
    countColor = '#27ae60';
  } else {
    countColor = '#e74c3c';
  }
}

export function onInc() {
  count = count + 1;
  porfSyncCountColor();
  porfPatch_count();
  porfPatchStyle_countColor_color();
}

export function onDec() {
  count = count - 1;
  porfSyncCountColor();
  porfPatch_count();
  porfPatchStyle_countColor_color();
}

export function onReset() {
  count = 0;
  porfSyncCountColor();
  porfPatch_count();
  porfPatchStyle_countColor_color();
}

porfInit();
