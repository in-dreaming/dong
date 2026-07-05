var g_outsideBody = "";

export function onLocalData() {
  var rid = fetchRequestId();
  var ok = fetchOk();
  var status = fetchStatus();
  var body = fetchBody();
  dongLog("LOCAL:" + rid + ":" + ok + ":" + status + ":" + body);
}

export function onError() {
  var ok = fetchOk();
  var status = fetchStatus();
  var err = fetchError();
  dongLog("ERR:" + ok + ":" + status + ":" + err);
}

export function onConcurrent() {
  dongLog("CONC:" + fetchRequestId() + ":" + fetchStatus() + ":" + fetchBody());
}

export function onAborted() {
  dongLog("ABORT_SHOULD_NOT_RUN");
}

export function probeOutsideSlot() {
  g_outsideBody = fetchBody();
  dongLog("OUTSIDE:" + g_outsideBody + ":" + String(fetchRequestId()));
}

export function runFetchTests() {
  dongLog("T10_INIT");
  dongFetch("good.json", "onLocalData");
  dongFetch("missing.json", "onError");
  dongFetch("bad://not-a-url", "onError");
  var id1 = dongFetch("a.json", "onConcurrent");
  var id2 = dongFetch("b.json", "onConcurrent");
  var abortId = dongFetch("slow.json", "onAborted");
  fetchAbort(abortId);
  dongStateSetNum(0, id1);
  dongStateSetNum(1, id2);
}

function main() {
}
