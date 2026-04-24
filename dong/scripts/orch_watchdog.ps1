param(
    [int]$TickSleepSeconds = 90,
    [int]$CliHealthMinutes = 15
)

$ErrorActionPreference = "Continue"
# dong/scripts -> repo root (parent of dong/)
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repoRoot

Write-Output "[orch-watchdog] repo=$repoRoot tick_sleep=${TickSleepSeconds}s cli_health=${CliHealthMinutes}m"
python dong\scripts\orch.py cli-health
$nextCli = [datetime]::UtcNow.AddMinutes($CliHealthMinutes)

while ($true) {
    $ts = Get-Date -Format o
    Write-Output "[orch-watchdog] $ts tick"
    python dong\scripts\orch.py tick

    if (Test-Path "dong\.orchestration\snapshots\state.json") {
        $state = Get-Content "dong\.orchestration\snapshots\state.json" -Raw | ConvertFrom-Json
        $ids = @()
        $state.features.PSObject.Properties | ForEach-Object {
            if ($_.Value.status -eq "awaiting_review") { $ids += $_.Name }
        }
        foreach ($id in $ids) {
            Write-Output "[orch-watchdog] approving $id"
            python dong\scripts\orch.py approve $id
            python dong\scripts\orch.py tick
        }
    }

    if ([datetime]::UtcNow -ge $nextCli) {
        $hts = Get-Date -Format o
        Write-Output "[orch-watchdog] $hts cli-health (every ${CliHealthMinutes} min)"
        python dong\scripts\orch.py cli-health
        $nextCli = [datetime]::UtcNow.AddMinutes($CliHealthMinutes)
    }

    Start-Sleep -Seconds $TickSleepSeconds
}
