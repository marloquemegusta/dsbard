$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$logDir = Join-Path $root '.tools\desmume-logs'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$log = Join-Path $logDir "loop-host-trace-$stamp.log"
$exe = Join-Path $root 'tests\loop_trace.exe'

New-Item -ItemType Directory -Force -Path $logDir | Out-Null
Push-Location $root
try {
    & g++ -std=c++17 -Iinclude 'tests/loop_trace.cpp' -o $exe 2>&1 | Tee-Object -FilePath $log -Append
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo compilar la traza host' }
    & $exe 2>&1 | Tee-Object -FilePath $log -Append
    if ($LASTEXITCODE -ne 0) { throw 'La traza host del looper falló' }
}
finally {
    Pop-Location
}

Write-Output "LOG=$log"
