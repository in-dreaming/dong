var intervalCount = 0;
var intervalTimerId = 0;
var rafCount = 0;
var rafId = 0;
var lastRafTs = 0;
var cancelProbeId = 0;
var cancelProbeCount = 0;

export function onIntervalTick() {
  intervalCount = intervalCount + 1;
  dong_state_set_num(0, intervalCount);
  if (intervalCount >= 5) {
    clearInterval(intervalTimerId);
  }
}

export function onRafFrame() {
  var ts = rafTimestamp();
  if (ts >= lastRafTs) {
    lastRafTs = ts;
  }
  rafCount = rafCount + 1;
  dong_state_set_num(1, rafCount);
  dong_state_set_num(2, ts);
  if (rafCount < 60) {
    requestAnimationFrame("onRafFrame");
  }
}

export function onRafWithArg(ts) {
  dong_state_set_num(3, ts);
}

export function onCancelProbe() {
  cancelProbeCount = cancelProbeCount + 1;
  dong_state_set_num(10, cancelProbeCount);
}

intervalTimerId = setInterval("onIntervalTick", 100);
rafId = requestAnimationFrame("onRafFrame");
requestAnimationFrame("onRafWithArg");
cancelProbeId = requestAnimationFrame("onCancelProbe");
cancelAnimationFrame(cancelProbeId);
dong_state_set_num(4, intervalTimerId);
dong_state_set_num(5, rafId);
