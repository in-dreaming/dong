param(
    [int]$CliHealthSeconds = 30,
    [int]$TailLines = 25
)

$ErrorActionPreference = "Continue"
# dong/scripts -> repo root (parent of dong/)
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repoRoot

Write-Output "[orch-watchdog] readonly monitor repo=$repoRoot cli_health=${CliHealthSeconds}s tail_lines=${TailLines}"
python dong\scripts\orch.py cli-health --dry-run --tail-lines $TailLines

while ($true) {
    $ts = Get-Date -Format o
    Write-Output "[orch-watchdog] $ts cli-health (every ${CliHealthSeconds}s)"
    python dong\scripts\orch.py cli-health --dry-run --tail-lines $TailLines
    Start-Sleep -Seconds $CliHealthSeconds
}
