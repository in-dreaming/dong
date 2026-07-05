# Generate T13-test-inventory.md (fallback when node unavailable)
$ErrorActionPreference = 'Stop'
$dongRoot = Split-Path $PSScriptRoot -Parent
$repoRoot = Split-Path $dongRoot -Parent
$testsDir = Join-Path $dongRoot 'examples\data\tests'
$outPath = Join-Path $repoRoot 'docs\developer\porffor\tasks\T13-test-inventory.md'

function Get-PorfforTag($html) {
    if ($html -match '<!--\s*porffor:\s*ready\s*-->') { return @{ status='ready'; reason=''; explicit=$true } }
    if ($html -match '<!--\s*porffor:\s*pending\s*-->') { return @{ status='pending'; reason=''; explicit=$true } }
    if ($html -match '<!--\s*porffor:\s*blocked\(([^)]+)\)\s*-->') { return @{ status='blocked'; reason=$Matches[1]; explicit=$true } }
    if ($html -match '<!--\s*porffor:\s*dropped\(([^)]+)\)\s*-->') { return @{ status='dropped'; reason=$Matches[1]; explicit=$true } }
    $hasScript = $html -match '<script[\s>]'
    if ($hasScript) { return @{ status='pending'; reason=''; explicit=$false } }
    return @{ status='ready'; reason=''; explicit=$false }
}

function Get-MigrationBucket($html, $fileName) {
    $lower = $html.ToLower()
    $name = $fileName.ToLower()
    if ($name -match 'react|preact') { return 'blocked(T18)' }
    if ($lower -match 'execcommand|getselection|createrange|contenteditable') { return 'blocked(T20)' }
    if ($html -match 'eval-after-frame0-file') { return 'T14-snippet' }
    if ($lower -match 'onclick\s*=|onchange\s*=') { return 'blocked(T12)' }
    if ($lower -match '\beval\s*\(|\basync\b|\bpromise\b') { return 'blocked(T19)' }
    if ($lower -match 'formdata|domparser|fetch\s*\(') { return 'blocked(T20)' }
    if ($html -notmatch '<script[\s>]') { return 'static-render' }
    if ($lower -match 'console\.log|getelementbyid|textcontent') { return 'direct' }
    return 'pending-review'
}

$rows = @()
$counts = @{ ready=0; pending=0; blocked=0; dropped=0 }
$buckets = @{}

Get-ChildItem $testsDir -Filter *.html | Sort-Object Name | ForEach-Object {
    $html = Get-Content $_.FullName -Raw
    $tag = Get-PorfforTag $html
    $counts[$tag.status]++
    $hasScript = $html -match '<script[\s>]'
    $mod = if ($html -match 'data-porffor-module="([^"]+)"') { $Matches[1] } else { '' }
    $migration = Get-MigrationBucket $html $_.Name
    if (-not $buckets.ContainsKey($migration)) { $buckets[$migration] = 0 }
    $buckets[$migration]++
    $rows += [pscustomobject]@{
        file = $_.Name
        status = $tag.status
        explicit = if ($tag.explicit) { 'yes' } else { 'no' }
        reason = $tag.reason
        hasScript = if ($hasScript) { 'yes' } else { 'no' }
        module = $mod
        migration = $migration
    }
}

$total = $rows.Count
$readyPct = [math]::Round(100.0 * $counts.ready / $total, 1)
$bucketLines = ($buckets.GetEnumerator() | Sort-Object Value -Descending | ForEach-Object { "| $($_.Key) | $($_.Value) |" }) -join "`n"
$tableLines = ($rows | ForEach-Object { "| $($_.file) | $($_.status) | $($_.explicit) | $($_.reason) | $($_.hasScript) | $($_.module) | $($_.migration) |" }) -join "`n"

$md = @"
# T13 — 全量测试迁移盘点

> 自动生成：``node dong/scripts/porffor_test_inventory.mjs`` 或 ``dong/scripts/generate_t13_inventory.ps1``

## 摘要

| 指标 | 值 |
|------|-----|
| 测试总数 | $total |
| ready | $($counts.ready) ($readyPct%) |
| pending | $($counts.pending) |
| blocked | $($counts.blocked) |
| dropped | $($counts.dropped) |
| 无 script（静态渲染） | $(($rows | Where-Object hasScript -eq 'no').Count) |
| 有 script | $(($rows | Where-Object hasScript -eq 'yes').Count) |

## 构建组织选型（阶段 0 定稿）

**选定方案：a) 单 test runner 链接全部 Porffor 模块**

| 方案 | 说明 | 结论 |
|------|------|------|
| a) 单 runner + 全量 registry | manifest → ``porffor_compile.mjs`` → ``generated/porffor/registry.c`` | **采用** |
| b) 按目录分组多 registry | 多二进制 | 暂缓 |
| c) 每测试独立编译 + 缓存 | 增量友好 | 不采用 |

**实测**：~11 模块时 ``porffor_compile.mjs`` <3s；全量 150 模块预估 compile <30s，CI 可接受。

## 迁移分类（script 测试）

| 分类 | 数量 |
|------|------|
$bucketLines

## 全量清单

| 文件 | porffor 状态 | 显式标记 | 阻塞原因 | script | 模块 | 迁移分类 |
|------|-------------|---------|---------|--------|------|---------|
$tableLines

## dropped 候选（待评审）

| 文件 | 建议理由 |
|------|---------|
| complex.html | 综合演示页，非回归粒度 |
| skin_test.html | 皮肤/主题实验，非引擎契约 |

## T14 snippet 交叉引用

见 [T14-snippet-inventory.md](./T14-snippet-inventory.md)。
"@

New-Item -ItemType Directory -Force -Path (Split-Path $outPath) | Out-Null
Set-Content -Path $outPath -Value $md -Encoding UTF8
Write-Host "[generate_t13_inventory] $total tests -> $outPath"
