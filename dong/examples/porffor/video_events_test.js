var videoId = 0;
var logId = 0;
var logText = '';
var cLoadedmetadata = 0;
var cLoadeddata = 0;
var cCanplay = 0;
var cPlay = 0;
var cPlaying = 0;
var cPause = 0;
var cTimeupdate = 0;
var cEnded = 0;
var cError = 0;

function appendLine(text) {
  if (logText == '') {
    logText = text;
  } else {
    logText = logText + '\n' + text;
  }
  setTextContent(logId, logText);
}

function videoLine(name) {
  var ct = getAttribute(videoId, '__dong_video_currentTime');
  var dur = getAttribute(videoId, '__dong_video_duration');
  appendLine(name + ' ct=' + ct + ' dur=' + dur);
}

function toggleVideo() {
  var playing = getAttribute(videoId, '__dong_video_playing');
  if (playing == '1' || playing == 'true') {
    setAttribute(videoId, '__dong_video_playing', '0');
  } else {
    setAttribute(videoId, '__dong_video_playing', '1');
  }
}

export function onLoadedmetadata() {
  cLoadedmetadata = cLoadedmetadata + 1;
  setTextContent(getElementById('c_loadedmetadata'), '' + cLoadedmetadata);
  videoLine('loadedmetadata');
}

export function onLoadeddata() {
  cLoadeddata = cLoadeddata + 1;
  setTextContent(getElementById('c_loadeddata'), '' + cLoadeddata);
  videoLine('loadeddata');
}

export function onCanplay() {
  cCanplay = cCanplay + 1;
  setTextContent(getElementById('c_canplay'), '' + cCanplay);
  videoLine('canplay');
}

export function onPlay() {
  cPlay = cPlay + 1;
  setTextContent(getElementById('c_play'), '' + cPlay);
  videoLine('play');
}

export function onPlaying() {
  cPlaying = cPlaying + 1;
  setTextContent(getElementById('c_playing'), '' + cPlaying);
  videoLine('playing');
}

export function onPause() {
  cPause = cPause + 1;
  setTextContent(getElementById('c_pause'), '' + cPause);
  videoLine('pause');
}

export function onTimeupdate() {
  cTimeupdate = cTimeupdate + 1;
  setTextContent(getElementById('c_timeupdate'), '' + cTimeupdate);
  videoLine('timeupdate');
}

export function onEnded() {
  cEnded = cEnded + 1;
  setTextContent(getElementById('c_ended'), '' + cEnded);
  videoLine('ended');
}

export function onError() {
  cError = cError + 1;
  setTextContent(getElementById('c_error'), '' + cError);
  videoLine('error');
}

export function onVideoClick() {
  toggleVideo();
}

function porfInit() {
  videoId = getElementById('v');
  logId = getElementById('log');

  addEventListener(videoId, 'loadedmetadata', 'onLoadedmetadata');
  addEventListener(videoId, 'loadeddata', 'onLoadeddata');
  addEventListener(videoId, 'canplay', 'onCanplay');
  addEventListener(videoId, 'play', 'onPlay');
  addEventListener(videoId, 'playing', 'onPlaying');
  addEventListener(videoId, 'pause', 'onPause');
  addEventListener(videoId, 'timeupdate', 'onTimeupdate');
  addEventListener(videoId, 'ended', 'onEnded');
  addEventListener(videoId, 'error', 'onError');
  addEventListener(videoId, 'click', 'onVideoClick');

  appendLine('listeners installed');
  dongLog('video_events_test loaded');
}

porfInit();
