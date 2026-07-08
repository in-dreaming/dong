var fpsValueId = 0;
var helpPanelId = 0;
var helpHintId = 0;
var helpVisible = 0;

function initHud() {
  fpsValueId = getElementById('fps-value');
  helpPanelId = getElementById('help-panel');
  helpHintId = getElementById('help-hint');
  setHelpVisibleInternal(0);
}

function setHelpVisibleInternal(visible) {
  helpVisible = visible ? 1 : 0;
  if (helpVisible) {
    classAdd(helpPanelId, 'visible');
    setStyle(helpPanelId, 'opacity', '1');
    setStyle(helpHintId, 'opacity', '0');
  } else {
    classRemove(helpPanelId, 'visible');
    setStyle(helpPanelId, 'opacity', '0');
    setStyle(helpHintId, 'opacity', '1');
  }
}

export function setFps(fps) {
  setTextContent(fpsValueId, numToStr(fps));
}

export function toggleHelp() {
  setHelpVisibleInternal(helpVisible ? 0 : 1);
}

initHud();
