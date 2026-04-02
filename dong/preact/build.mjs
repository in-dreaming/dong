import { build } from 'esbuild';
import { mkdirSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));

const example = process.argv[2];
if (!example) {
    console.error('Usage: node build.mjs <example-name>');
    console.error('  e.g. node build.mjs counter');
    process.exit(1);
}

const entryPoint = join(__dirname, 'examples', example, 'main.jsx');
const outfile = join(__dirname, 'dist', example, 'bundle.js');

mkdirSync(dirname(outfile), { recursive: true });

await build({
    entryPoints: [entryPoint],
    bundle: true,
    format: 'iife',
    outfile,
    minify: false,
    target: 'es2020',
    jsx: 'automatic',
    jsxImportSource: 'preact',
    define: {
        'process.env.NODE_ENV': '"production"',
    },
    logLevel: 'info',
});

console.log(`Built: ${outfile}`);
