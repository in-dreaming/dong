#!/usr/bin/env node
/**
 * Capture Preact Chrome baselines for T18 porf examples (no Python required).
 * Uses Playwright via dynamic import; run: node scripts/tools/capture_porf_preact_baseline.mjs
 */
import fs from 'node:fs';
import path from 'node:path';
import http from 'node:http';
import { fileURLToPath } from 'node:url';
import { createRequire } from 'node:module';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const dongRoot = path.resolve(__dirname, '../..');
const dataDir = path.join(dongRoot, 'zig-out/bin/data');
const outRoot = path.resolve(
  dongRoot,
  '../docs/developer/porffor/tasks/repro/t18/baseline',
);

const CASES = [
  { name: 'counter', preact: 'preact-counter' },
  { name: 'todo-classic', preact: 'preact-todo-classic' },
  { name: 'game-ui', preact: 'preact-game-ui' },
];

const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json',
  '.png': 'image/png',
  '.bmp': 'image/bmp',
};

function startStaticServer(root) {
  return new Promise((resolve) => {
    const server = http.createServer((req, res) => {
      const rel = decodeURIComponent(req.url.split('?')[0]).replace(/^\//, '') || 'index.html';
      const file = path.normalize(path.join(root, rel));
      if (!file.startsWith(root)) {
        res.writeHead(403);
        res.end();
        return;
      }
      fs.readFile(file, (err, data) => {
        if (err) {
          res.writeHead(404);
          res.end();
          return;
        }
        const ext = path.extname(file).toLowerCase();
        res.writeHead(200, { 'Content-Type': MIME[ext] ?? 'application/octet-stream' });
        res.end(data);
      });
    });
    server.listen(0, '127.0.0.1', () => {
      const { port } = server.address();
      resolve({ server, port });
    });
  });
}

async function loadPlaywright() {
  try {
    const require = createRequire(import.meta.url);
    return require('playwright');
  } catch {
    const mod = await import('playwright');
    return mod.default ?? mod;
  }
}

async function main() {
  if (!fs.existsSync(dataDir)) {
    console.error(`Missing ${dataDir} — run zig build preact first`);
    process.exit(2);
  }

  let playwright;
  try {
    playwright = await loadPlaywright();
  } catch (e) {
    console.error('Playwright not installed. Run: npm install -D playwright && npx playwright install chromium');
    console.error(e.message);
    process.exit(2);
  }

  const { server, port } = await startStaticServer(dataDir);
  const browser = await playwright.chromium.launch();
  const page = await browser.newPage({ viewport: { width: 1024, height: 768 } });

  try {
    for (const c of CASES) {
      const outDir = path.join(outRoot, c.name);
      fs.mkdirSync(outDir, { recursive: true });
      const outPng = path.join(outDir, 'preact_f0.png');
      const url = `http://127.0.0.1:${port}/${c.preact}/index.html`;
      console.log(`[capture] ${c.name} <- ${url}`);
      await page.goto(url, { waitUntil: 'networkidle', timeout: 30000 });
      await page.waitForTimeout(550);
      await page.screenshot({ path: outPng, fullPage: false });
      console.log(`  -> ${outPng}`);
    }
  } finally {
    await browser.close();
    server.close();
  }

  console.log('capture_porf_preact_baseline: done');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
