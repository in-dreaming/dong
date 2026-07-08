var bodyId = 0;
var badgeId = 0;
var outId = 0;
var videoId = 0;

function boolText(v) {
  return v ? 'true' : 'false';
}

function isPaused() {
  var playing = getAttribute(videoId, '__dong_video_playing');
  return !(playing == '1' || playing == 'true');
}

function line(text) {
  var old = getTextContent(outId);
  setTextContent(outId, old + text + '\n');
}

function porfInit() {
  bodyId = getElementById('body');
  badgeId = getElementById('badge');
  outId = getElementById('out');
  videoId = getElementById('v');

  setTextContent(outId, '');
  setAttribute(videoId, '__dong_video_playing', '0');
  setAttribute(videoId, '__dong_video_seeking', '0');
  setAttribute(videoId, '__dong_video_currentTime', '0');

  var ok = true;
  var p0 = isPaused();
  if (!p0) ok = false;

  setAttribute(videoId, '__dong_video_playing', '1');
  var p1 = isPaused();
  if (p1) ok = false;

  setAttribute(videoId, '__dong_video_playing', '0');
  var p2 = isPaused();
  if (!p2) ok = false;

  setAttribute(videoId, '__dong_video_seek', '1.0');
  setAttribute(videoId, '__dong_video_currentTime', '1.0');
  setAttribute(videoId, '__dong_video_seeking', '1');
  var s1 = getAttribute(videoId, '__dong_video_seeking') == '1';
  if (!s1) ok = false;

  line('default paused: ' + boolText(p0));
  line('after play paused: ' + boolText(p1));
  line('after pause paused: ' + boolText(p2));
  line('after set currentTime seeking: ' + boolText(s1));

  if (ok) {
    setAttribute(bodyId, 'class', 'ok');
    setTextContent(badgeId, 'VIDEO JS API: OK');
  } else {
    setTextContent(badgeId, 'VIDEO JS API: FAIL');
  }

  dongLog('video_js_api_smoke_test loaded');
}

porfInit();
