var tab1Id = 0;
var tab2Id = 0;
var tab3Id = 0;
var tab4Id = 0;
var tab5Id = 0;
var tab6Id = 0;
var page1Id = 0;
var page2Id = 0;
var page3Id = 0;
var page4Id = 0;
var page5Id = 0;
var page6Id = 0;

function clearTabs() {
  classRemove(tab1Id, 'active');
  classRemove(tab2Id, 'active');
  classRemove(tab3Id, 'active');
  classRemove(tab4Id, 'active');
  classRemove(tab5Id, 'active');
  classRemove(tab6Id, 'active');

  classRemove(page1Id, 'active');
  classRemove(page2Id, 'active');
  classRemove(page3Id, 'active');
  classRemove(page4Id, 'active');
  classRemove(page5Id, 'active');
  classRemove(page6Id, 'active');

  setStyle(page1Id, 'display', 'none');
  setStyle(page2Id, 'display', 'none');
  setStyle(page3Id, 'display', 'none');
  setStyle(page4Id, 'display', 'none');
  setStyle(page5Id, 'display', 'none');
  setStyle(page6Id, 'display', 'none');
}

function activateTab(tabId, pageId) {
  clearTabs();
  classAdd(tabId, 'active');
  classAdd(pageId, 'active');
  setStyle(pageId, 'display', 'block');
}

export function onTab1() {
  activateTab(tab1Id, page1Id);
}

export function onTab2() {
  activateTab(tab2Id, page2Id);
}

export function onTab3() {
  activateTab(tab3Id, page3Id);
}

export function onTab4() {
  activateTab(tab4Id, page4Id);
}

export function onTab5() {
  activateTab(tab5Id, page5Id);
}

export function onTab6() {
  activateTab(tab6Id, page6Id);
}

function porfInit() {
  tab1Id = getElementById('tab-1');
  tab2Id = getElementById('tab-2');
  tab3Id = getElementById('tab-3');
  tab4Id = getElementById('tab-4');
  tab5Id = getElementById('tab-5');
  tab6Id = getElementById('tab-6');
  page1Id = getElementById('page-1');
  page2Id = getElementById('page-2');
  page3Id = getElementById('page-3');
  page4Id = getElementById('page-4');
  page5Id = getElementById('page-5');
  page6Id = getElementById('page-6');

  addEventListener(tab1Id, 'click', 'onTab1');
  addEventListener(tab2Id, 'click', 'onTab2');
  addEventListener(tab3Id, 'click', 'onTab3');
  addEventListener(tab4Id, 'click', 'onTab4');
  addEventListener(tab5Id, 'click', 'onTab5');
  addEventListener(tab6Id, 'click', 'onTab6');

  onTab1();
  dongLog('feature_test loaded');
}

porfInit();
