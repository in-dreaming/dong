export function probeEventSlot() {
  dongLog(
    eventType() +
      ";" +
      String(eventTarget()) +
      ";" +
      eventKey() +
      ";" +
      String(eventKeyCode()) +
      ";" +
      String(eventX()) +
      "," +
      String(eventY()) +
      ";" +
      String(eventButton()) +
      ";" +
      String(eventModifiers()) +
      ";" +
      eventValue(),
  );
}

export function markPrevent() {
  preventDefault();
  stopPropagation();
}

export function readTypeDuringNested() {
  dongLog("NESTED:" + eventType());
}

function main() {
  dongLog("T08_INIT");
}
