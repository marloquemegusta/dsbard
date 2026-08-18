[CmdletBinding()]
param(
    [int]$RunSeconds = 4,
    [switch]$ManualTty
)

$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$smokeDir = Join-Path $root '.tools\nogba-smoke'
$debugDir = Join-Path $root '.tools\nogba-debug'
$rom = Join-Path $smokeDir 'nogba-smoke.nds'
$exe = Get-ChildItem -LiteralPath $debugDir -Filter '*GBA.EXE' | Select-Object -First 1 -ExpandProperty FullName

if (-not $exe) { throw "NO$GBA executable not found in $debugDir" }

docker run --rm -v "${smokeDir}:/source" -w /source skylyrac/blocksds:slim-latest make
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $rom)) {
    Write-Output 'NOGBA_SMOKE_RESULT=FAIL reason=blocksds_build'
    exit 1
}

# Start-Process does not quote paths with spaces on its own.
$runProcess = Start-Process -FilePath $exe -WorkingDirectory $debugDir -ArgumentList ('"' + $rom + '"') -PassThru
if (-not ('NogbaAutomation' -as [type])) {
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class NogbaAutomation {
  public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lParam);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);
  [DllImport("user32.dll")] public static extern IntPtr GetMenu(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern IntPtr GetSubMenu(IntPtr hMenu, int pos);
  [DllImport("user32.dll")] public static extern int GetMenuItemCount(IntPtr hMenu);
  [DllImport("user32.dll")] public static extern uint GetMenuItemID(IntPtr hMenu, int pos);
  [DllImport("user32.dll")] public static extern bool SendMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);
}
'@
}

Start-Sleep -Seconds 2
$windows = [System.Collections.Generic.List[object]]::new()
$collectWindows = [NogbaAutomation+EnumWindowsProc]{ param($hWnd, $lParam)
    [uint32]$windowPid = 0
    [NogbaAutomation]::GetWindowThreadProcessId($hWnd, [ref]$windowPid) | Out-Null
    if ($windowPid -eq $runProcess.Id -and [NogbaAutomation]::IsWindowVisible($hWnd)) {
        $title = [System.Text.StringBuilder]::new(256)
        [NogbaAutomation]::GetWindowText($hWnd, $title, $title.Capacity) | Out-Null
        $windows.Add([PSCustomObject]@{ Handle = $hWnd; Title = $title.ToString() })
    }
    return $true
}
[NogbaAutomation]::EnumWindows($collectWindows, [IntPtr]::Zero) | Out-Null
$debugger = $windows | Where-Object Title -like '*Debugger*' | Select-Object -First 1
if (-not $debugger) {
    if (-not $runProcess.HasExited) { Stop-Process -Id $runProcess.Id -Force; $runProcess.WaitForExit() }
    Write-Output 'NOGBA_SMOKE_RESULT=FAIL reason=debugger_window_missing'
    exit 1
}

# The debugger starts paused. Invoke its Run menu action instead of relying on
# foreground focus or synthetic keyboard input.
$runMenu = [NogbaAutomation]::GetSubMenu([NogbaAutomation]::GetMenu($debugger.Handle), 2)
$runCommand = [NogbaAutomation]::GetMenuItemID($runMenu, 0)
[NogbaAutomation]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$runCommand, [IntPtr]::Zero) | Out-Null
$ttyOutput = ''

if ($ManualTty) {
    # Prevent a previous TTY copy from being mistaken for evidence from this run.
    Set-Clipboard -Value "NOGBA_SMOKE_PENDING_$([guid]::NewGuid().ToString('N'))"
    Write-Output 'NOGBA_SMOKE_TTY=MANUAL_COPY_REQUIRED'
    Write-Output 'In NO$GBA: Window > TTY Debug Messages > Copy. Then return here and press Enter.'
    Read-Host | Out-Null
    $ttyOutput = Get-Clipboard -Raw
} else {
# NO$GBA 3.06 exposes messages in a TTY window. Copy its text through the
# emulator's own Copy command; this yields terminal-verifiable evidence without
# screenshots or relying on the legacy DEBUGMSG.TXT file.
$ttyCommand = [NogbaAutomation]::GetMenuItemID(
    [NogbaAutomation]::GetSubMenu([NogbaAutomation]::GetMenu($debugger.Handle), 4), 25)
[NogbaAutomation]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$ttyCommand, [IntPtr]::Zero) | Out-Null
Start-Sleep -Milliseconds 500
Start-Sleep -Seconds $RunSeconds
$windows.Clear()
[NogbaAutomation]::EnumWindows($collectWindows, [IntPtr]::Zero) | Out-Null
$tty = $windows | Where-Object Title -like '*TTY Debug*' | Select-Object -First 1
$ttyCopyInvoked = $false
if ($tty) {
    $copyCommand = [NogbaAutomation]::GetMenuItemID([NogbaAutomation]::GetMenu($tty.Handle), 2)
    $ttyCopyInvoked = [NogbaAutomation]::SendMessage($tty.Handle, 0x0111, [IntPtr]$copyCommand, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 250
    $ttyOutput = Get-Clipboard -Raw
}
}
if (-not $runProcess.HasExited) { Stop-Process -Id $runProcess.Id -Force; $runProcess.WaitForExit() }

if ($ttyOutput -match 'SMOKE\|STDERR\|boot' -and $ttyOutput -match 'SMOKE\|NOCASH\|boot') {
    Write-Output 'NOGBA_SMOKE_RESULT=PASS'
    exit 0
}

Write-Output "NOGBA_TTY_DIAGNOSTIC manual=$ManualTty window=$([bool]$tty) copy_command=$ttyCopyInvoked stderr_marker=$($ttyOutput -match 'SMOKE\|STDERR\|boot') nocash_marker=$($ttyOutput -match 'SMOKE\|NOCASH\|boot')"
Write-Output 'NOGBA_SMOKE_RESULT=FAIL reason=tty_messages_missing_or_incomplete'
exit 1
