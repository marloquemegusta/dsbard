[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$smokeDir = Join-Path $root '.tools\nogba-smoke'
$debugDir = Join-Path $root '.tools\nogba-debug'
$rom = Join-Path $smokeDir 'nogba-smoke.nds'
$exe = Get-ChildItem -LiteralPath $debugDir -Filter '*GBA.EXE' |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $exe) { throw "NO$GBA executable not found in $debugDir" }

docker run --rm -v "${smokeDir}:/source" -w /source skylyrac/blocksds:slim-latest make
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $rom)) { throw 'BlocksDS smoke build failed' }

if (-not ('NogbaSmokeWindow' -as [type])) {
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class NogbaSmokeWindow {
  public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lParam);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);
  [DllImport("user32.dll")] public static extern IntPtr GetMenu(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern IntPtr GetSubMenu(IntPtr hMenu, int pos);
  [DllImport("user32.dll")] public static extern uint GetMenuItemID(IntPtr hMenu, int pos);
  [DllImport("user32.dll")] public static extern bool SendMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);
}
'@
}

# Quote the ROM path: the workspace path contains spaces.
$process = Start-Process -FilePath $exe -WorkingDirectory $debugDir -ArgumentList ('"' + $rom + '"') -PassThru
Start-Sleep -Seconds 2

$windows = [System.Collections.Generic.List[object]]::new()
$collect = [NogbaSmokeWindow+EnumWindowsProc]{ param($hWnd, $lParam)
    [uint32]$windowPid = 0
    [NogbaSmokeWindow]::GetWindowThreadProcessId($hWnd, [ref]$windowPid) | Out-Null
    if ($windowPid -eq $process.Id -and [NogbaSmokeWindow]::IsWindowVisible($hWnd)) {
        $title = [System.Text.StringBuilder]::new(256)
        [NogbaSmokeWindow]::GetWindowText($hWnd, $title, $title.Capacity) | Out-Null
        $windows.Add([PSCustomObject]@{ Handle = $hWnd; Title = $title.ToString() })
    }
    return $true
}
[NogbaSmokeWindow]::EnumWindows($collect, [IntPtr]::Zero) | Out-Null
$debugger = $windows | Where-Object Title -like '*Debugger*' | Select-Object -First 1
if (-not $debugger) { throw 'NO$GBA debugger window did not open' }

$menu = [NogbaSmokeWindow]::GetMenu($debugger.Handle)
$runCommand = [NogbaSmokeWindow]::GetMenuItemID([NogbaSmokeWindow]::GetSubMenu($menu, 2), 0)
$ttyCommand = [NogbaSmokeWindow]::GetMenuItemID([NogbaSmokeWindow]::GetSubMenu($menu, 4), 25)
[NogbaSmokeWindow]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$runCommand, [IntPtr]::Zero) | Out-Null
Start-Sleep -Milliseconds 500
[NogbaSmokeWindow]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$ttyCommand, [IntPtr]::Zero) | Out-Null

Write-Output "NOGBA_SMOKE_TTY=OPEN pid=$($process.Id)"
Write-Output 'Inspect Window > TTY Debug Messages for SMOKE|STDERR|boot and SMOKE|NOCASH|boot.'
