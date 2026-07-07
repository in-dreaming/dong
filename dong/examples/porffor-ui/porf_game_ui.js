var hud_rootId = 0;
var hp_label_valId = 0;
var hp_fillId = 0;
var mp_label_valId = 0;
var mp_fillId = 0;
var btn_damageId = 0;
var btn_healId = 0;
var score_mainId = 0;



function porfInit() {
  hud_rootId = getElementById('hud-root');
  hp_label_valId = getElementById('hp-label-val');
  hp_fillId = getElementById('hp-fill');
  mp_label_valId = getElementById('mp-label-val');
  mp_fillId = getElementById('mp-fill');
  btn_damageId = getElementById('btn-damage');
  btn_healId = getElementById('btn-heal');
  score_mainId = getElementById('score-main');
  addEventListener(btn_damageId, 'click', 'onDamage');
  addEventListener(btn_healId, 'click', 'onHeal');
  porfRefresh();
  dongLog('porfInit');
}

var hp = 85;
var mp = 60;
var score = 12750;
var highScore = 50000;
var scoreTimerId = 0;
var scoreTick = 0;

function hpBarColor(pct) {
  if (pct > 60) {
    return '#27ae60';
  }
  if (pct > 30) {
    return '#f39c12';
  }
  return '#e74c3c';
}

function porfPatchHp() {
  var pct = hp;
  if (pct < 0) { pct = 0; }
  if (pct > 100) { pct = 100; }
  setStyle(hp_fillId, 'width', String(pct) + '%');
  setStyle(hp_fillId, 'background-color', hpBarColor(pct));
  setTextContent(hp_label_valId, String(hp) + '/100');
}

function porfPatchMp() {
  var pct = mp;
  if (pct < 0) { pct = 0; }
  if (pct > 100) { pct = 100; }
  setStyle(mp_fillId, 'width', String(pct) + '%');
  setStyle(mp_fillId, 'background-color', hpBarColor(pct));
  setTextContent(mp_label_valId, String(mp) + '/100');
}

function porfPatchScore() {
  setTextContent(score_mainId, String(score));
}

function porfRefresh() {
  porfPatchHp();
  porfPatchMp();
  porfPatchScore();
  if (scoreTimerId === 0) {
    scoreTimerId = setInterval('onScoreTick', 1000);
  }
}

export function onDamage() {
  hp = hp - 10;
  if (hp < 0) {
    hp = 0;
  }
  porfPatchHp();
}

export function onHeal() {
  hp = hp + 15;
  if (hp > 100) {
    hp = 100;
  }
  porfPatchHp();
}

export function onScoreTick() {
  scoreTick = scoreTick + 1;
  var bump = 5 + (scoreTick % 5);
  score = score + bump;
  porfPatchScore();
}

porfInit();
