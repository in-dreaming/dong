var vAutoId = 0;
var vPauseId = 0;

function toggleVideo(videoId) {
  var playing = getAttribute(videoId, '__dong_video_playing');
  if (playing == '1' || playing == 'true') {
    setAttribute(videoId, '__dong_video_playing', '0');
  } else {
    setAttribute(videoId, '__dong_video_playing', '1');
  }
}

export function onAutoClick() {
  toggleVideo(vAutoId);
}

export function onPauseClick() {
  toggleVideo(vPauseId);
}

function porfInit() {
  vAutoId = getElementById('v_auto');
  vPauseId = getElementById('v_pause');
  addEventListener(vAutoId, 'click', 'onAutoClick');
  addEventListener(vPauseId, 'click', 'onPauseClick');
  dongLog('video_play_test loaded');
}

porfInit();
