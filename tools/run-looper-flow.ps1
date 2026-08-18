param(
    [int]$Seconds = 3,
    [switch]$EnableSound
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$logDir = Join-Path $root '.tools\desmume-logs'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$log = Join-Path $logDir "looper-flow-$stamp.log"
$testExe = Join-Path $root 'tests\test_loop_logic.exe'

New-Item -ItemType Directory -Force -Path $logDir | Out-Null

function Log([string]$line) {
    $line | Tee-Object -FilePath $log -Append
}

Log "LOOPER_FLOW_START=$stamp"
Log 'AUDIO=DISABLED_BY_DEFAULT'

Push-Location $root
try {
    Log 'STEP=PYTHON_HOST_TEST'
    & python 'tests/test_loop_logic.py' 2>&1 | ForEach-Object { Log ([string]$_) }
    if ($LASTEXITCODE -ne 0) { throw 'Python host test failed' }
    Log 'PYTHON_HOST_TEST=PASS'

    Log 'STEP=CPP_HOST_TEST'
    & g++ -std=c++17 -Iinclude 'tests/test_loop_logic.cpp' -o $testExe 2>&1 | ForEach-Object { Log ([string]$_) }
    if ($LASTEXITCODE -ne 0) { throw 'C++ host test compilation failed' }
    & $testExe 2>&1 | ForEach-Object { Log ([string]$_) }
    if ($LASTEXITCODE -ne 0) { throw 'C++ host test failed' }
    Log 'CPP_HOST_TEST=PASS'

    Log 'STEP=HOST_EVENT_TRACE'
    $traceOutput = & (Join-Path $root 'tools\run-loop-host-trace.ps1') 2>&1
    $traceOutput | ForEach-Object { Log ([string]$_) }
    if ($LASTEXITCODE -ne 0 -or -not ($traceOutput -match 'TRACE\|RESULT\|PASS')) {
        throw 'La traza host del looper no pasó'
    }
    Log 'HOST_EVENT_TRACE=PASS'

    Log 'STEP=DSM_REPLAY'
    & (Join-Path $root 'tools\create-looper-movie.ps1') 2>&1 | ForEach-Object { Log ([string]$_) }
    $playtestParams = @{
        Seconds = $Seconds
        Movie = Join-Path $root 'tools\looper-three-notes.dsm'
    }
    if ($EnableSound) { $playtestParams.EnableSound = $true }
    $playtestOutput = & (Join-Path $root 'tools\run-desmume-playtest.ps1') @playtestParams 2>&1
    $playtestOutput | ForEach-Object { Log ([string]$_) }
    $running = $playtestOutput -match 'RUNNING_BEFORE_STOP=True'
    if (-not $running) { throw 'DeSmuME no permaneció ejecutando la ROM durante el replay' }
    Log 'DSM_REPLAY=PASS'
    Log 'LOOPER_FLOW_RESULT=PASS'
}
catch {
    Log ('LOOPER_FLOW_RESULT=FAIL: ' + $_.Exception.Message)
    throw
}
finally {
    Pop-Location
}

Write-Output "LOG=$log"
