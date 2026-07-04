var statusId = getElementById('status');
var btnId = getElementById('btn');
addEventListener(btnId, 'click', 'onBtnClick');
setTextContent(statusId, 'ready');
dongLog('hello_dom loaded');

export function onBtnClick() {
  setTextContent(statusId, 'clicked');
}
