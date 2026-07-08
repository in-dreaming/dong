var linesId = 0;

function porfInit() {
  linesId = getElementById('lines');
  var obstacles =
    '{"circles":[{"cx":205,"cy":200,"r":60,"hPad":10,"vPad":4},{"cx":660,"cy":140,"r":40,"hPad":10,"vPad":4}],"rects":[{"x":580,"y":280,"w":110,"h":80,"hPad":10,"vPad":4}]}';
  var columns = pretextFlowColumnsStatic();
  var cfg = pretextTypoConfig(columns, obstacles);
  var lineCount = textLayoutMountLines(linesId, cfg, 'line', 1);
  dongLog('pretext_text_flow mounted');
  dongLog(numToStr(lineCount));
}

porfInit();
