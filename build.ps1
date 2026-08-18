$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$testDir = Join-Path $root '.tools\blocksds-test'
$isolatedRom = Join-Path $testDir 'ds-stylophone-blocksds.nds'
$rom = Join-Path $root 'ds-stylophone.nds'

Copy-Item -LiteralPath (Join-Path $root 'source\main.cpp') -Destination (Join-Path $testDir 'arm9\source\main.cpp') -Force
Copy-Item -LiteralPath (Join-Path $root 'include\loop_logic.h') -Destination (Join-Path $testDir 'arm9\include\loop_logic.h') -Force
New-Item -ItemType Directory -Force -Path (Join-Path $testDir 'arm9\assets\samples') | Out-Null
Copy-Item -Path (Join-Path $root 'assets\samples\*.bin') -Destination (Join-Path $testDir 'arm9\assets\samples') -Force
docker run --rm -v "${testDir}:/source" -w /source skylyrac/blocksds:slim-latest make
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $isolatedRom)) { throw 'BlocksDS build failed.' }

Copy-Item -LiteralPath $isolatedRom -Destination $rom -Force
[System.IO.File]::Delete($isolatedRom)
Write-Host "ROM: $rom"
