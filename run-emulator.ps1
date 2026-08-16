$ErrorActionPreference = 'Stop'
$emulator = Join-Path $PSScriptRoot '.tools\melonds\melonDS.exe'
$rom = Join-Path $PSScriptRoot 'ds-stylophone.nds'
if (-not (Test-Path -LiteralPath $emulator)) { throw "melonDS no está disponible en $emulator" }
if (-not (Test-Path -LiteralPath $rom)) { throw "Compila primero la ROM: $rom" }
Start-Process -FilePath $emulator -ArgumentList @('"' + $rom + '"')
