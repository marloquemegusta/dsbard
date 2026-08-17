$ErrorActionPreference = 'Stop'
$emulator = Join-Path $PSScriptRoot '.tools\nogba-debug\NO$GBA.EXE'
$rom = Join-Path $PSScriptRoot '.tools\blocksds-test\ds-stylophone-blocksds.nds'
if (-not (Test-Path -LiteralPath $emulator)) { throw "NO$GBA is not installed at $emulator" }
if (-not (Test-Path -LiteralPath $rom)) { throw "Build the BlocksDS ROM first: $rom" }

# NO$GBA needs the ROM path quoted because the workspace path contains spaces.
Start-Process -FilePath $emulator -WorkingDirectory (Split-Path -Parent $emulator) -ArgumentList ('"' + $rom + '"')
