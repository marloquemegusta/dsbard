$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$movie = Join-Path $root 'tools\looper-three-notes.dsm'
$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add('version 2')
$lines.Add('emuVersion 0')
$lines.Add('rerecordCount 0')
$lines.Add('romFilename ds-stylophone.nds')
$lines.Add('romChecksum 00000000')
$lines.Add('romSerial ')
$lines.Add('guid 452DE2C3-EF43-2FA9-77AC-0677FC51543B')
$lines.Add('useExtBios 0')
$lines.Add('useExtFirmware 0')
$lines.Add('firmNickname ')
$lines.Add('firmMessage ')
$lines.Add('firmFavColour 0')
$lines.Add('firmBirthMonth 1')
$lines.Add('firmBirthDay 1')
$lines.Add('firmLanguage 1')
$lines.Add('advancedTiming 1')
$lines.Add('jitBlockSize 0')
$lines.Add('rtcStartNew 2009-01-01T00:00:00Z')
$lines.Add('comment looper-three-notes')
$lines.Add('savestate 0')

function Add-Frame([string]$buttons, [int]$x = 0, [int]$y = 0, [int]$touch = 0) {
    $columns = [char[]]'.............'
    foreach ($button in $buttons.ToCharArray()) {
        $index = 'RLDUTSBAYXWEG'.IndexOf($button)
        if ($index -ge 0) { $columns[$index] = $button }
    }
    $lines.Add('|0|' + (-join $columns) + ('{0:D3} {1:D3} {2:D1} {3:D3}' -f $x, $y, $touch, 0) + '|')
}

1..60 | ForEach-Object { Add-Frame '' }
Add-Frame 'A'
1..10 | ForEach-Object { Add-Frame '' }
1..25 | ForEach-Object { Add-Frame '' 10 100 1 }
1..25 | ForEach-Object { Add-Frame '' 95 100 1 }
1..25 | ForEach-Object { Add-Frame '' 160 100 1 }
1..20 | ForEach-Object { Add-Frame '' }
Add-Frame 'A'
1..600 | ForEach-Object { Add-Frame '' }

[System.IO.File]::WriteAllLines($movie, $lines)
Write-Output $movie
