var t = 0;

function porfInit() {
  pretextDomOnlyInit();
  setInterval('tick', 16);
  dongLog('pretext_dual_mode_domonly loaded');
}

export function tick() {
  t = t + 1;
  pretextDomOnlyTick(t);
}

porfInit();
