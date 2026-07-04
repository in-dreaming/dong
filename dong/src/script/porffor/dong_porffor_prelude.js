// Porffor prelude — flat helpers only (no object method calls).
// Multi-arg host calls use single-arg stage + commit imports (Porffor 2c limitation).

function getElementById(id) {
  return dong_dom_getElementById(id);
}

function setTextContent(nodeId, text) {
  dong_stage_0(nodeId);
  dong_stage_1(text);
  dong_commit_set_textContent();
}

function addEventListener(nodeId, type, handlerName) {
  dong_stage_0(nodeId);
  dong_stage_1(type);
  dong_stage_2(handlerName);
  dong_commit_addEventListener();
}

function dongLog(msg) {
  dong_print(msg);
}

function benchLog(msg) {
  dong_bench_log(msg);
}

function setTimeout(handlerName, ms) {
  dong_stage_0(ms);
  dong_stage_1(handlerName);
  dong_commit_setTimeout();
}

function dongStateSetNum(slot, v) {
  dong_state_set_num(slot, v);
}

function dongStateGetNum(slot) {
  return dong_state_get_num(slot);
}
