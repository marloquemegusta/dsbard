[CmdletBinding()]
param(
    # Launch a fresh NO$GBA process with the canonical ROM before capturing.
    [switch]$Launch,
    # Rebuild the canonical ROM before launching it. Requires -Launch.
    [switch]$Build,
    # PNG destination. Parent directories are created when needed.
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\artifacts\nogba\screen.png'),
    # Seconds to wait for NO$GBA windows to become available.
    [ValidateRange(1, 30)]
    [int]$StartupTimeoutSeconds = 5,
    # Windows monitor number (1-based) where NO$GBA must run and be captured.
    [ValidateRange(1, 16)]
    [int]$MonitorNumber = 2,
    # Close only the NO$GBA process launched by this invocation after capture.
    [switch]$CloseLaunched
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'nogba-debug-common.ps1')

if ($Build -and -not $Launch) {
    throw '-Build requires -Launch.'
}

if (-not ('NogbaScreenCapture' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;

public static class NogbaScreenCapture {
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc callback, IntPtr lParam);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int maxCount);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT point);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr GetMenu(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr GetSubMenu(IntPtr menu, int position);
    [DllImport("user32.dll")] public static extern uint GetMenuItemID(IntPtr menu, int position);
    [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr hWnd, uint message, IntPtr wParam, IntPtr lParam);
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
'@
}

$root = Split-Path -Parent $PSScriptRoot
$launchedProcess = $null

try {
    if ($Build) {
        & (Join-Path $root 'build.ps1')
        if ($LASTEXITCODE -ne 0) { throw 'Canonical BlocksDS build failed.' }
    }

    if ($Launch) {
        $emulator = Join-Path $root '.tools\nogba-debug\NO$GBA.EXE'
        $rom = Join-Path $root 'ds-stylophone.nds'
        if (-not (Test-Path -LiteralPath $emulator)) { throw "NO`$GBA is not installed at $emulator" }
        if (-not (Test-Path -LiteralPath $rom)) { throw "Canonical ROM is missing at $rom" }
        $launchedProcess = Start-Process -FilePath $emulator -WorkingDirectory (Split-Path -Parent $emulator) -ArgumentList ('"' + $rom + '"') -PassThru
    }

    $deadline = (Get-Date).AddSeconds($StartupTimeoutSeconds)
    $gameWindow = $null
    $debuggerWindow = $null
    do {
        $windows = [System.Collections.Generic.List[object]]::new()
        $collect = [NogbaScreenCapture+EnumWindowsProc]{ param($handle, $unused)
            [uint32]$windowProcessId = 0
            [NogbaScreenCapture]::GetWindowThreadProcessId($handle, [ref]$windowProcessId) | Out-Null
            if ([NogbaScreenCapture]::IsWindowVisible($handle)) {
                $title = [Text.StringBuilder]::new(512)
                [NogbaScreenCapture]::GetWindowText($handle, $title, $title.Capacity) | Out-Null
                $windows.Add([pscustomobject]@{ Handle = $handle; Pid = $windowProcessId; Title = $title.ToString() })
            }
            return $true
        }
        [NogbaScreenCapture]::EnumWindows($collect, [IntPtr]::Zero) | Out-Null

        if ($launchedProcess) {
            $candidateWindows = $windows | Where-Object Pid -eq $launchedProcess.Id
        } else {
            $candidateWindows = $windows | Where-Object { $_.Title -match 'NO\$GBA.*(Emulator|Debugger)' }
        }
        $gameWindow = $candidateWindows | Where-Object Title -match 'Emulator' | Select-Object -First 1
        $debuggerWindow = $candidateWindows | Where-Object Title -match 'Debugger' | Select-Object -First 1
        if ($gameWindow) { break }
        Start-Sleep -Milliseconds 100
    } while ((Get-Date) -lt $deadline)

    if (-not $gameWindow) {
        throw "NO`$GBA Emulator window not found. Start it first, or use -Launch."
    }

    $monitor = Get-NogbaMonitor -Number $MonitorNumber
    Move-NogbaWindowsToMonitor -Windows $candidateWindows -Monitor $monitor
    Write-Output "NOGBA_MONITOR number=$MonitorNumber work=$($monitor.Work.Left),$($monitor.Work.Top),$($monitor.Work.Right),$($monitor.Work.Bottom)"

    # A newly launched Debugger starts paused. Invoke Run exactly as the existing
    # NO$GBA validation scripts do, then allow the first frame to be presented.
    if ($launchedProcess -and $debuggerWindow) {
        $runMenu = [NogbaScreenCapture]::GetSubMenu([NogbaScreenCapture]::GetMenu($debuggerWindow.Handle), 2)
        $runCommand = [NogbaScreenCapture]::GetMenuItemID($runMenu, 0)
        [NogbaScreenCapture]::SendMessage($debuggerWindow.Handle, 0x0111, [IntPtr]$runCommand, [IntPtr]::Zero) | Out-Null
        Start-Sleep -Milliseconds 750
        # NO$GBA may recreate/reposition the Emulator window when execution
        # starts. Apply the monitor placement again after Run.
        $runningWindows = $windows | Where-Object Pid -eq $launchedProcess.Id
        Move-NogbaWindowsToMonitor -Windows $runningWindows -Monitor $monitor
        Start-Sleep -Milliseconds 150
    }

    if ([NogbaScreenCapture]::IsIconic($gameWindow.Handle)) {
        throw 'The NO$GBA Emulator window is minimized. Restore it and retry; screen capture reads the displayed pixels.'
    }

    Start-Sleep -Milliseconds 150
    [NogbaScreenCapture+RECT]$client = New-Object NogbaScreenCapture+RECT
    if (-not [NogbaScreenCapture]::GetClientRect($gameWindow.Handle, [ref]$client)) { throw 'Unable to read NO$GBA client bounds.' }
    $clientWidth = $client.Right - $client.Left
    $clientHeight = $client.Bottom - $client.Top
    if ($clientWidth -eq 256 -and $clientHeight -eq 384) {
        $cropX = 0; $cropY = 0
    } elseif ($clientWidth -eq 260 -and $clientHeight -eq 392) {
        $cropX = 2; $cropY = 4
    } else {
        throw "Unexpected NO`$GBA client size ${clientWidth}x${clientHeight}. Set the Emulator window to native 1x stacked DS screens, then retry."
    }
    $width = 256
    $height = 384
    $origin = New-Object NogbaScreenCapture+POINT
    if (-not [NogbaScreenCapture]::ClientToScreen($gameWindow.Handle, [ref]$origin)) { throw 'Unable to convert NO$GBA client bounds to screen coordinates.' }

    Add-Type -AssemblyName System.Drawing
    $bitmap = [System.Drawing.Bitmap]::new($width, $height)
    try {
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
            # CopyFromScreen captures the pixels actually shown by NO$GBA; unlike
            # PrintWindow it does not silently create blank captures for this app.
            $graphics.CopyFromScreen($origin.X + $cropX, $origin.Y + $cropY, 0, 0, $bitmap.Size)
        } finally { $graphics.Dispose() }

        $output = if ([IO.Path]::IsPathRooted($OutputPath)) {
            [IO.Path]::GetFullPath($OutputPath)
        } else {
            [IO.Path]::GetFullPath((Join-Path $root $OutputPath))
        }
        $outputDir = Split-Path -Parent $output
        [IO.Directory]::CreateDirectory($outputDir) | Out-Null
        $bitmap.Save($output, [System.Drawing.Imaging.ImageFormat]::Png)
    } finally { $bitmap.Dispose() }

    $image = [System.Drawing.Image]::FromFile($output)
    try {
        if ($image.Width -ne 256 -or $image.Height -ne 384) { throw "Capture verification failed: expected 256x384, got $($image.Width)x$($image.Height)." }
    } finally { $image.Dispose() }
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $output).Hash.ToLowerInvariant()
    Write-Output "NOGBA_CAPTURE_RESULT=PASS path=$output size=256x384 sha256=$hash"
} finally {
    if ($CloseLaunched -and $launchedProcess -and -not $launchedProcess.HasExited) {
        Stop-Process -Id $launchedProcess.Id
        $launchedProcess.WaitForExit()
    }
}
