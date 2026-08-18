param(
    [int]$Seconds = 8,
    [string]$Movie = '',
    [switch]$EnableSound
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$emulator = Join-Path $root '.tools\desmume-stable\DeSmuME_0.9.13_x64.exe'
$rom = Join-Path $root 'ds-stylophone.nds'
$logDir = Join-Path $root '.tools\desmume-logs'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdout = Join-Path $logDir "playtest-$stamp.stdout.log"
$stderr = Join-Path $logDir "playtest-$stamp.stderr.log"

if (-not (Test-Path -LiteralPath $emulator)) { throw "DeSmuME no está instalado en $emulator" }
if (-not (Test-Path -LiteralPath $rom)) { throw "Compila primero la ROM: $rom" }
New-Item -ItemType Directory -Force -Path $logDir | Out-Null

function Get-ShortPath([string]$path) {
    $short = (cmd.exe /d /c ('for %I in ("' + $path + '") do @echo %~sI') | Select-Object -Last 1).Trim()
    if ([string]::IsNullOrWhiteSpace($short)) { throw "No se pudo obtener la ruta 8.3 de $path" }
    return $short
}

$startInfo = New-Object System.Diagnostics.ProcessStartInfo
$startInfo.FileName = $emulator
$startInfo.WorkingDirectory = Split-Path -Parent $emulator
$romArg = Get-ShortPath $rom
$arguments = if ([string]::IsNullOrWhiteSpace($Movie)) {
    '"' + $romArg + '"'
} else {
    if (-not (Test-Path -LiteralPath $Movie)) { throw "No existe el movie DSM: $Movie" }
    $movieArg = Get-ShortPath (Resolve-Path -LiteralPath $Movie).Path
    '--play-movie "' + $movieArg + '" "' + $romArg + '"'
}
if (-not $EnableSound) { $arguments = '--disable-sound ' + $arguments }
$startInfo.Arguments = $arguments
$startInfo.UseShellExecute = $false
$startInfo.CreateNoWindow = $true
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true
$process = New-Object System.Diagnostics.Process
$process.StartInfo = $startInfo
$process.Start() | Out-Null

try {
    Start-Sleep -Seconds $Seconds
    $process.Refresh()
    $runningBeforeStop = -not $process.HasExited
    $titleBeforeStop = $process.MainWindowTitle
    $exitCodeBeforeStop = if ($process.HasExited) { $process.ExitCode } else { '' }
}
finally {
    if (-not $process.HasExited) {
        $process.Kill()
        $process.WaitForExit()
    }
}

[System.IO.File]::WriteAllText($stdout, $process.StandardOutput.ReadToEnd())
[System.IO.File]::WriteAllText($stderr, $process.StandardError.ReadToEnd())

Write-Output "STDOUT=$stdout"
Write-Output "STDERR=$stderr"
Write-Output "RUNNING_BEFORE_STOP=$runningBeforeStop"
Write-Output "TITLE_BEFORE_STOP=$titleBeforeStop"
Write-Output "EXIT_CODE_BEFORE_STOP=$exitCodeBeforeStop"
if (Test-Path -LiteralPath $stdout) { Get-Content -LiteralPath $stdout }
if (Test-Path -LiteralPath $stderr) { Get-Content -LiteralPath $stderr }
