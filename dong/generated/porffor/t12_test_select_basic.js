var __dongLogSelect = 0;
var colorId = 0;
var countryId = 0;
var fruitId = 0;

export function country__onmousedown() {
  __dongLogSelect('[mousedown]', this)
}

export function country__onchange() {
  __dongLogSelect('[change]', this)
}

export function color__onmousedown() {
  __dongLogSelect('[mousedown]', this)
}

export function color__onchange() {
  __dongLogSelect('[change]', this)
}

export function fruit__onmousedown() {
  __dongLogSelect('[mousedown]', this)
}

export function fruit__onchange() {
  __dongLogSelect('[change]', this)
}

colorId = getElementById('color');
countryId = getElementById('country');
fruitId = getElementById('fruit');
addEventListener(countryId, 'mousedown', 'country__onmousedown');
addEventListener(countryId, 'change', 'country__onchange');
addEventListener(colorId, 'mousedown', 'color__onmousedown');
addEventListener(colorId, 'change', 'color__onchange');
addEventListener(fruitId, 'mousedown', 'fruit__onmousedown');
addEventListener(fruitId, 'change', 'fruit__onchange');
