$ErrorActionPreference = 'Stop'
docker run --rm -v "${PWD}:/source" -w /source devkitpro/devkitarm:latest make
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "ROM: $((Resolve-Path '.\ds-stylophone.nds').Path)"

