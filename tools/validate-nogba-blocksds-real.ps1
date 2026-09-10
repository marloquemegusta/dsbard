[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$testDir = Join-Path $root '.tools\blocksds-test'
$debugDir = Join-Path $root '.tools\nogba-debug'
$rom = Join-Path $root 'ds-stylophone.nds'
$exe = Get-ChildItem -LiteralPath $debugDir -Filter '*GBA.EXE' |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $exe) { throw "NO$GBA executable not found in $debugDir" }

& (Join-Path $root 'build.ps1')
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $rom)) {
    Write-Output 'NOGBA_BLOCKSDS_REAL_RESULT=FAIL reason=canonical_blocksds_build'
    exit 1
}

if (-not ('NogbaBlocksdsRealAutomation' -as [type])) {
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class NogbaBlocksdsRealAutomation {
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

# The workspace path contains spaces, so the ROM argument must be quoted.
$process = Start-Process -FilePath $exe -WorkingDirectory $debugDir -ArgumentList ('"' + $rom + '"') -PassThru
Start-Sleep -Seconds 2

$windows = [System.Collections.Generic.List[object]]::new()
$collect = [NogbaBlocksdsRealAutomation+EnumWindowsProc]{ param($hWnd, $lParam)
    [uint32]$windowPid = 0
    [NogbaBlocksdsRealAutomation]::GetWindowThreadProcessId($hWnd, [ref]$windowPid) | Out-Null
    if ($windowPid -eq $process.Id -and [NogbaBlocksdsRealAutomation]::IsWindowVisible($hWnd)) {
        $title = [System.Text.StringBuilder]::new(256)
        [NogbaBlocksdsRealAutomation]::GetWindowText($hWnd, $title, $title.Capacity) | Out-Null
        $windows.Add([PSCustomObject]@{ Handle = $hWnd; Title = $title.ToString() })
    }
    return $true
}
[NogbaBlocksdsRealAutomation]::EnumWindows($collect, [IntPtr]::Zero) | Out-Null
$debugger = $windows | Where-Object Title -like '*Debugger*' | Select-Object -First 1
if (-not $debugger) {
    if (-not $process.HasExited) { Stop-Process -Id $process.Id -Force; $process.WaitForExit() }
    Write-Output 'NOGBA_BLOCKSDS_REAL_RESULT=FAIL reason=debugger_window_missing'
    exit 1
}

$runCommand = [NogbaBlocksdsRealAutomation]::GetMenuItemID(
    [NogbaBlocksdsRealAutomation]::GetSubMenu(
        [NogbaBlocksdsRealAutomation]::GetMenu($debugger.Handle), 2), 0)
[NogbaBlocksdsRealAutomation]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$runCommand, [IntPtr]::Zero) | Out-Null

# A fresh clipboard value prevents a previous run's messages from passing.
Set-Clipboard -Value "NOGBA_BLOCKSDS_REAL_PENDING_$([guid]::NewGuid().ToString('N'))"
Write-Output 'NOGBA_BLOCKSDS_REAL=MANUAL_VISUAL_AND_TTY_REQUIRED'
Write-Output 'Confirm the stylophone UI renders in No$gba Emulator.'
Write-Output 'Then in No$gba: Window > TTY Debug Messages > Copy; return here and press Enter.'
Read-Host | Out-Null
$ttyOutput = Get-Clipboard -Raw

if (-not $process.HasExited) { Stop-Process -Id $process.Id -Force; $process.WaitForExit() }

if ($ttyOutput -match 'DSDBG\|BOOT\|trace_ready') {
    Write-Output 'NOGBA_BLOCKSDS_REAL_RESULT=PASS'
    exit 0
}

Write-Output "NOGBA_BLOCKSDS_REAL_DIAGNOSTIC boot_marker=$($ttyOutput -match 'DSDBG\|BOOT\|trace_ready')"
Write-Output 'NOGBA_BLOCKSDS_REAL_RESULT=FAIL reason=tty_messages_missing_or_incomplete'
exit 1
