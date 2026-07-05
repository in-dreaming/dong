export function main() {
  var formId = getElementById('f');
  var json = formSerialize(formId);
  if (json.indexOf('user') < 0 || json.indexOf('alice') < 0) {
    dongLog('T20_FAIL:form');
    return 1;
  }
  dongLog('T20_PASS:form');
  return 0;
}
