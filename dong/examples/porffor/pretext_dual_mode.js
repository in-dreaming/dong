var currentMode = 0;
var t = 0;
var sidebarId = 0;
var panelHeaderId = 0;
var panelDescId = 0;
var sidebarItem0 = 0;
var sidebarItem1 = 0;
var sidebarItem2 = 0;
var sidebarItem3 = 0;
var sidebarItem4 = 0;

function setActiveSidebar(activeId) {
  classRemove(sidebarItem0, 'active');
  classRemove(sidebarItem1, 'active');
  classRemove(sidebarItem2, 'active');
  classRemove(sidebarItem3, 'active');
  classRemove(sidebarItem4, 'active');
  classAdd(activeId, 'active');
}

function porfInit() {
  sidebarId = getElementById('sidebar');
  panelHeaderId = querySelector(0, '.panel-header');
  panelDescId = querySelector(0, '.panel-desc');
  sidebarItem0 = querySelector(sidebarId, '[data-mode="dashboard"]');
  sidebarItem1 = querySelector(sidebarId, '[data-mode="textflow"]');
  sidebarItem2 = querySelector(sidebarId, '[data-mode="particles"]');
  sidebarItem3 = querySelector(sidebarId, '[data-mode="charts"]');
  sidebarItem4 = querySelector(sidebarId, '[data-mode="settings"]');
  addEventListener(sidebarItem0, 'click', 'onModeDashboard');
  addEventListener(sidebarItem1, 'click', 'onModeTextflow');
  addEventListener(sidebarItem2, 'click', 'onModeParticles');
  addEventListener(sidebarItem3, 'click', 'onModeCharts');
  addEventListener(sidebarItem4, 'click', 'onModeSettings');
  setInterval('tick', 16);
  dongLog('pretext_dual_mode loaded');
}

export function onModeDashboard() {
  currentMode = 0;
  setActiveSidebar(sidebarItem0);
  setTextContent(panelHeaderId, 'Dual-Mode Rendering Engine');
  setTextContent(panelDescId, 'Static DOM (retained) + Dynamic overlay (immediate) — zero DOM tax');
}

export function onModeTextflow() {
  currentMode = 1;
  setActiveSidebar(sidebarItem1);
  setTextContent(panelHeaderId, 'Text Flow with Obstacles');
  setTextContent(panelDescId, 'Text wraps around moving circular obstacles — all via overlay API');
}

export function onModeParticles() {
  currentMode = 2;
  setActiveSidebar(sidebarItem2);
  setTextContent(panelHeaderId, 'Particle System');
  setTextContent(panelDescId, '120 particles rendered via dong.drawCircle() — zero DOM elements');
}

export function onModeCharts() {
  currentMode = 3;
  setActiveSidebar(sidebarItem3);
  setTextContent(panelHeaderId, 'Real-Time Charts');
  setTextContent(panelDescId, 'Animated bar chart rendered via dong.drawRect() — pure immediate mode');
}

export function onModeSettings() {
  currentMode = 4;
  setActiveSidebar(sidebarItem4);
  setTextContent(panelHeaderId, 'Architecture Info');
  setTextContent(panelDescId, 'Retained DOM: menus, panels | Immediate: particles, text, charts');
}

export function tick() {
  t = t + 1;
  pretextDualModeTick(currentMode, t);
}

porfInit();
