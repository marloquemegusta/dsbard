[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [string]$HostName = '192.168.1.151',
    [int]$Port = 5000,
    [string]$RemoteDirectory = '/roms/nds',
    [string]$RemoteName = 'ds-stylophone.nds',
    [string]$User = 'anonymous',
    [string]$Password = ''
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$rom = Join-Path $root 'ds-stylophone.nds'

if (-not $SkipBuild) {
    & (Join-Path $root 'build.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'El build canónico BlocksDS falló.' }
}

$romItem = Get-Item -LiteralPath $rom -ErrorAction Stop
$curl = Get-Command curl.exe -ErrorAction Stop
$ftpBase = "ftp://$HostName`:$Port$RemoteDirectory"
$remoteUrl = "$ftpBase/$RemoteName"

Write-Host "Subiendo $($romItem.FullName) a $remoteUrl"
& $curl.Source --connect-timeout 8 --max-time 60 --fail --show-error `
    --ftp-pasv --user "$User`:$Password" `
    --upload-file $romItem.FullName $remoteUrl
if ($LASTEXITCODE -ne 0) {
    throw "La subida FTP falló (curl exit code $LASTEXITCODE)."
}

$listing = & $curl.Source --connect-timeout 8 --max-time 20 --fail --silent --show-error `
    --ftp-pasv --user "$User`:$Password" "$ftpBase/"
if ($LASTEXITCODE -ne 0) {
    throw 'La subida terminó, pero no se pudo verificar el listado FTP.'
}

$escapedName = [regex]::Escape($RemoteName)
if (-not ($listing -match "(?m)$escapedName\s*$")) {
    throw "La ROM no aparece en $ftpBase después de la subida."
}

Write-Host 'FTP_UPLOAD_RESULT=PASS'
Write-Host "REMOTE=$remoteUrl"
Write-Host "LOCAL_BYTES=$($romItem.Length)"
