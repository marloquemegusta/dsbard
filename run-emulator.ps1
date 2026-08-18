$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$emulator = Join-Path $PSScriptRoot '.tools\nogba-debug\NO$GBA.EXE'
$rom = Join-Path $root 'ds-stylophone.nds'
if (-not (Test-Path -LiteralPath $emulator)) { throw "NO$GBA is not installed at $emulator" }

& (Join-Path $root 'build.ps1')
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $rom)) { throw "ROM build failed: $rom" }

# NO$GBA needs the ROM path quoted because the workspace path contains spaces.
Start-Process -FilePath $emulator -WorkingDirectory (Split-Path -Parent $emulator) -ArgumentList ('"' + $rom + '"')
