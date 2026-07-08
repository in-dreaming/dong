var hudId = 0;
var obsAId = 0;
var obsBId = 0;
var obsCId = 0;

var t = 0;
var frameCount = 0;
var fps = 0;
var lastFpsTime = 0;

function porfInit() {
  hudId = getElementById('hud');
  obsAId = getElementById('obs-a');
  obsBId = getElementById('obs-b');
  obsCId = getElementById('obs-c');
  setInterval('tick', 16);
  tick();
  dongLog('pretext_text_flow_directdraw loaded');
}

export function tick() {
  t = t + 1;
  var now = nowMs();
  if (lastFpsTime === 0) {
    lastFpsTime = now;
  }
  frameCount = frameCount + 1;
  if (now - lastFpsTime >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFpsTime = now;
  }

  pretextFlowDynamicTick(1, 0, hudId, obsAId, obsBId, obsCId, t, fps);
}

porfInit();
