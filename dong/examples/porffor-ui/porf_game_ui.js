var hp = 85;
var mp = 60;
var score = 12750;
var highScore = 50000;
var scoreTimerId = 0;

var hpFillId = 0;
var mpFillId = 0;
var hpLabelId = 0;
var mpLabelId = 0;
var scoreMainId = 0;
var btnDamageId = 0;
var btnHealId = 0;

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
  setStyle(hpFillId, 'width', String(pct) + '%');
  setStyle(hpFillId, 'background-color', hpBarColor(pct));
  setTextContent(hpLabelId, String(hp) + '/100');
}

function porfPatchMp() {
  var pct = mp;
  if (pct < 0) { pct = 0; }
  if (pct > 100) { pct = 100; }
  setStyle(mpFillId, 'width', String(pct) + '%');
  setStyle(mpFillId, 'background-color', hpBarColor(pct));
  setTextContent(mpLabelId, String(mp) + '/100');
}

function porfPatchScore() {
  setTextContent(scoreMainId, String(score));
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
  var bump = 5;
  score = score + bump;
  porfPatchScore();
}

hpFillId = getElementById('hp-fill');
mpFillId = getElementById('mp-fill');
hpLabelId = getElementById('hp-label-val');
mpLabelId = getElementById('mp-label-val');
scoreMainId = getElementById('score-main');
btnDamageId = getElementById('btn-damage');
btnHealId = getElementById('btn-heal');

addEventListener(btnDamageId, 'click', 'onDamage');
addEventListener(btnHealId, 'click', 'onHeal');

porfPatchHp();
porfPatchMp();
porfPatchScore();

scoreTimerId = setInterval('onScoreTick', 1000);
dongLog('porf_game_ui loaded');
