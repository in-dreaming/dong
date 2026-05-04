#!/usr/bin/env tsx

/**
 * TypeScript 模块导入分析工具
 *
 * 功能：
 * - 扫描指定目录下的所有 TypeScript 文件
 * - 提取并分类导入语句
 * - 生成依赖关系报告
 * - 检测循环依赖和潜在问题
 */

import * as fs from 'fs';
import * as path from 'path';

// ============================================================================
// 类型定义
// ============================================================================

interface ImportInfo {
  /** 源文件路径 */
  sourceFile: string;
  /** 导入的模块路径 */
  importPath: string;
  /** 导入的标识符 */
  identifiers: string[];
  /** 是否为类型导入 */
  isTypeOnly: boolean;
  /** 导入类型：external, relative, alias, builtin */
  importType: 'external' | 'relative' | 'alias' | 'builtin';
  /** 原始导入语句 */
  rawStatement: string;
}

interface DependencyStats {
  /** 总文件数 */
  totalFiles: number;
  /** 总导入数 */
  totalImports: number;
  /** 外部依赖 */
  externalDeps: Map<string, number>;
  /** 内部模块依赖 */
  internalDeps: Map<string, number>;
  /** 类型导入数量 */
  typeImports: number;
  /** 所有导入信息 */
  imports: ImportInfo[];
}

interface CircularDependency {
  cycle: string[];
}

// ============================================================================
// 工具函数
// ============================================================================

/**
 * 判断是否为 Node.js 内置模块
 */
const builtinModules = new Set([
  'fs', 'path', 'http', 'https', 'crypto', 'util', 'os', 'events',
  'stream', 'buffer', 'child_process', 'cluster', 'url', 'querystring',
  'zlib', 'dns', 'net', 'tls', 'readline', 'repl', 'vm', 'assert',
]);

function isBuiltinModule(moduleName: string): boolean {
  return builtinModules.has(moduleName.replace(/^node:/, ''));
}

/**
 * 分类导入类型
 */
function classifyImport(importPath: string): ImportInfo['importType'] {
  if (isBuiltinModule(importPath)) {
    return 'builtin';
  }
  if (importPath.startsWith('.') || importPath.startsWith('/')) {
    return 'relative';
  }
  if (importPath.startsWith('@/')) {
    return 'alias';
  }
  return 'external';
}

/**
 * 提取包名（处理 scoped packages）
 */
function extractPackageName(importPath: string): string {
  if (importPath.startsWith('@')) {
    const parts = importPath.split('/');
    return `${parts[0]}/${parts[1]}`;
  }
  return importPath.split('/')[0];
}

/**
 * 解析 TypeScript 文件中的导入语句
 */
function parseImports(filePath: string, content: string): ImportInfo[] {
  const imports: ImportInfo[] = [];

  // 匹配各种导入语句的正则表达式
  const importPatterns = [
    // import ... from '...'
    /import\s+(?:type\s+)?(?:(?:\{[^}]*\}|\*\s+as\s+\w+|\w+)(?:\s*,\s*)?)+\s+from\s+['"]([^'"]+)['"]/g,
    // import type { ... } from '...'
    /import\s+type\s+\{[^}]*\}\s+from\s+['"]([^'"]+)['"]/g,
    // import '...'
    /import\s+['"]([^'"]+)['"]/g,
    // export ... from '...'
    /export\s+(?:\{[^}]*\}|\*)\s+from\s+['"]([^'"]+)['"]/g,
  ];

  for (const pattern of importPatterns) {
    let match;
    while ((match = pattern.exec(content)) !== null) {
      const importPath = match[1];
      const rawStatement = match[0];
      const isTypeOnly = rawStatement.includes('import type');

      // 提取导入的标识符
      const identifiers: string[] = [];
      const identifierMatch = rawStatement.match(/\{([^}]+)\}|import\s+(\w+)|as\s+(\w+)/);
      if (identifierMatch) {
        if (identifierMatch[1]) {
          // 解构导入
          identifiers.push(...identifierMatch[1].split(',').map(s => s.trim().replace(/\s+as\s+.*/, '')));
        } else if (identifierMatch[2]) {
          // 默认导入
          identifiers.push(identifierMatch[2]);
        } else if (identifierMatch[3]) {
          // namespace 导入
          identifiers.push(identifierMatch[3]);
        }
      }

      imports.push({
        sourceFile: filePath,
        importPath,
        identifiers: identifiers.filter(Boolean),
        isTypeOnly,
        importType: classifyImport(importPath),
        rawStatement,
      });
    }
  }

  return imports;
}

/**
 * 递归扫描目录获取所有文件
 */
function* walkDirectory(dir: string, excludePatterns: RegExp[]): Generator<string> {
  const entries = fs.readdirSync(dir, { withFileTypes: true });

  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);

    // 检查是否应该排除
    const shouldExclude = excludePatterns.some(pattern => pattern.test(fullPath));
    if (shouldExclude) {
      continue;
    }

    if (entry.isDirectory()) {
      yield* walkDirectory(fullPath, excludePatterns);
    } else if (entry.isFile() && /\.(ts|tsx|js|jsx)$/.test(entry.name)) {
      yield fullPath;
    }
  }
}

/**
 * 扫描目录下的所有 TypeScript 文件
 */
function scanDirectory(dirPath: string): DependencyStats {
  const stats: DependencyStats = {
    totalFiles: 0,
    totalImports: 0,
    externalDeps: new Map(),
    internalDeps: new Map(),
    typeImports: 0,
    imports: [],
  };

  // 排除模式
  const excludePatterns = [
    /node_modules/,
    /\/dist\//,
    /\/build\//,
    /\.test\.(ts|tsx|js|jsx)$/,
    /\.spec\.(ts|tsx|js|jsx)$/,
  ];

  // 查找所有 TypeScript 文件
  const files = Array.from(walkDirectory(dirPath, excludePatterns));
  stats.totalFiles = files.length;

  // 分析每个文件
  for (const file of files) {
    try {
      const content = fs.readFileSync(file, 'utf-8');
      const fileImports = parseImports(file, content);

      stats.imports.push(...fileImports);
      stats.totalImports += fileImports.length;

      for (const imp of fileImports) {
        if (imp.isTypeOnly) {
          stats.typeImports++;
        }

        if (imp.importType === 'external') {
          const pkgName = extractPackageName(imp.importPath);
          stats.externalDeps.set(pkgName, (stats.externalDeps.get(pkgName) || 0) + 1);
        } else if (imp.importType === 'relative' || imp.importType === 'alias') {
          stats.internalDeps.set(imp.importPath, (stats.internalDeps.get(imp.importPath) || 0) + 1);
        }
      }
    } catch (error) {
      // 忽略无法读取的文件
      console.warn(`⚠️  无法读取文件: ${file}`);
    }
  }

  return stats;
}

/**
 * 检测循环依赖（简化版本）
 */
function detectCircularDependencies(imports: ImportInfo[]): CircularDependency[] {
  const graph = new Map<string, Set<string>>();

  // 构建依赖图
  for (const imp of imports) {
    if (imp.importType === 'relative' || imp.importType === 'alias') {
      if (!graph.has(imp.sourceFile)) {
        graph.set(imp.sourceFile, new Set());
      }
      // 简化处理：将相对路径转换为可能的绝对路径
      const resolvedPath = path.resolve(path.dirname(imp.sourceFile), imp.importPath);
      graph.get(imp.sourceFile)!.add(resolvedPath);
    }
  }

  // DFS 检测环
  const cycles: CircularDependency[] = [];
  const visited = new Set<string>();
  const recStack = new Set<string>();

  function dfs(node: string, path: string[]): void {
    visited.add(node);
    recStack.add(node);
    path.push(node);

    const neighbors = graph.get(node);
    if (neighbors) {
      for (const neighbor of neighbors) {
        if (!visited.has(neighbor)) {
          dfs(neighbor, [...path]);
        } else if (recStack.has(neighbor)) {
          // 发现环
          const cycleStart = path.indexOf(neighbor);
          if (cycleStart !== -1) {
            cycles.push({ cycle: path.slice(cycleStart) });
          }
        }
      }
    }

    recStack.delete(node);
  }

  for (const node of graph.keys()) {
    if (!visited.has(node)) {
      dfs(node, []);
    }
  }

  return cycles;
}

/**
 * 生成报告
 */
function generateReport(stats: DependencyStats): void {
  console.log('\n📊 导入统计概览');
  console.log('='.repeat(60));
  console.log(`📁 分析的文件总数: ${stats.totalFiles}`);
  console.log(`📦 导入语句总数: ${stats.totalImports}`);
  console.log(`🌐 外部依赖数量: ${stats.externalDeps.size}`);
  console.log(`🔗 内部模块引用: ${stats.internalDeps.size}`);
  console.log(`📝 类型导入数量: ${stats.typeImports}`);

  // 外部依赖排名
  console.log('\n\n📦 外部依赖排名（Top 15）');
  console.log('='.repeat(60));
  const sortedExternal = Array.from(stats.externalDeps.entries())
    .sort((a, b) => b[1] - a[1])
    .slice(0, 15);

  console.log('排名 | 包名 | 引用次数');
  console.log('-----|------|----------');
  sortedExternal.forEach(([pkg, count], index) => {
    console.log(`${(index + 1).toString().padStart(4)} | ${pkg.padEnd(40)} | ${count}`);
  });

  // 内部模块引用排名
  console.log('\n\n🔗 内部模块引用排名（Top 15）');
  console.log('='.repeat(60));
  const sortedInternal = Array.from(stats.internalDeps.entries())
    .sort((a, b) => b[1] - a[1])
    .slice(0, 15);

  console.log('排名 | 模块路径 | 引用次数');
  console.log('-----|----------|----------');
  sortedInternal.forEach(([mod, count], index) => {
    console.log(`${(index + 1).toString().padStart(4)} | ${mod.padEnd(50)} | ${count}`);
  });

  // 检测深层相对导入
  console.log('\n\n⚠️ 潜在问题');
  console.log('='.repeat(60));

  const deepImports = stats.imports.filter(
    imp => imp.importType === 'relative' && (imp.importPath.match(/\.\.\//g) || []).length >= 3
  );

  if (deepImports.length > 0) {
    console.log(`\n❗ 发现 ${deepImports.length} 个深层相对路径导入（3层及以上）：`);
    deepImports.slice(0, 10).forEach(imp => {
      console.log(`  ${imp.sourceFile}`);
      console.log(`    → ${imp.importPath}`);
    });
  }

  // 循环依赖检测
  const cycles = detectCircularDependencies(stats.imports);
  if (cycles.length > 0) {
    console.log(`\n🔄 发现 ${cycles.length} 个潜在的循环依赖`);
    cycles.slice(0, 5).forEach((cycle, index) => {
      console.log(`\n循环 ${index + 1}:`);
      cycle.cycle.forEach(file => {
        console.log(`  → ${path.relative(process.cwd(), file)}`);
      });
    });
  }

  // 优化建议
  console.log('\n\n💡 优化建议');
  console.log('='.repeat(60));

  if (deepImports.length > 0) {
    console.log('1. 考虑配置路径别名（path alias）来替代深层相对导入');
  }

  if (cycles.length > 0) {
    console.log('2. 重构循环依赖，考虑提取共享逻辑到独立模块');
  }

  const typeImportRatio = (stats.typeImports / stats.totalImports) * 100;
  if (typeImportRatio < 20) {
    console.log(`3. 类型导入比例较低（${typeImportRatio.toFixed(1)}%），考虑使用 'import type' 优化编译性能`);
  }

  console.log('\n');
}

// ============================================================================
// 主程序
// ============================================================================

function main() {
  const targetPath = process.argv[2] || process.cwd();

  if (!fs.existsSync(targetPath)) {
    console.error(`❌ 路径不存在: ${targetPath}`);
    process.exit(1);
  }

  const absPath = path.resolve(targetPath);
  console.log(`\n🔍 开始分析: ${absPath}\n`);

  try {
    const stats = scanDirectory(absPath);
    generateReport(stats);
  } catch (error) {
    console.error('❌ 分析失败:', error);
    process.exit(1);
  }
}

main();
