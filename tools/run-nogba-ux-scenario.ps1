[CmdletBinding()]
param(
    # JSON file containing an ordered UX-debug scenario.
    [Parameter(Mandatory)]
    [string]$ScenarioPath,
    # Root directory for the scenario PNGs. Defaults to artifacts/nogba/<scenario name>.
    [string]$OutputDirectory = '',
    # Rebuild the canonical ROM before starting the scenario.
    [switch]$Build,
    # Keep the instance started for this scenario open after its last step.
    [switch]$KeepOpen
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$captureScript = Join-Path $PSScriptRoot 'capture-nogba-screen.ps1'
$scenarioFile = (Resolve-Path -LiteralPath $ScenarioPath).Path
$scenario = Get-Content -LiteralPath $scenarioFile -Raw | ConvertFrom-Json

if ([string]::IsNullOrWhiteSpace($scenario.name)) { throw 'Scenario JSON requires a non-empty "name".' }
if ($null -eq $scenario.steps -or $scenario.steps.Count -eq 0) { throw 'Scenario JSON requires at least one step.' }
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $root ('artifacts\nogba\' + $scenario.name)
}
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
[IO.Directory]::CreateDirectory($OutputDirectory) | Out-Null

if (-not ('NogbaScenarioInput' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class NogbaScenarioInput {
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc callback, IntPtr lParam);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int maxCount);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT point);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint flags, uint dx, uint dy, uint data, UIntPtr extraInfo);
}
'@
}

function Get-NogbaGameWindow {
    $script:nogbaScenarioWindow = [IntPtr]::Zero
    $collect = [NogbaScenarioInput+EnumWindowsProc]{ param($handle, $unused)
        if ([NogbaScenarioInput]::IsWindowVisible($handle)) {
            $title = [Text.StringBuilder]::new(512)
            [NogbaScenarioInput]::GetWindowText($handle, $title, $title.Capacity) | Out-Null
            if ($title.ToString() -match 'NO\$GBA.*Emulator') {
                $script:nogbaScenarioWindow = $handle
                return $false
            }
        }
        return $true
    }
    [NogbaScenarioInput]::EnumWindows($collect, [IntPtr]::Zero) | Out-Null
    if ($script:nogbaScenarioWindow -eq [IntPtr]::Zero) { throw 'NO$GBA Emulator window is not available.' }
    return $script:nogbaScenarioWindow
}

function Invoke-NogbaTouch([int]$X, [int]$Y, [int]$HoldMilliseconds = 120) {
    if ($X -lt 0 -or $X -gt 255 -or $Y -lt 192 -or $Y -gt 383) {
        throw "Touch coordinates must be in the DS touch screen (x=0..255, y=192..383); got x=$X y=$Y."
    }
    $window = Get-NogbaGameWindow
    [NogbaScenarioInput]::SetForegroundWindow($window) | Out-Null
    Start-Sleep -Milliseconds 100

    [NogbaScenarioInput+RECT]$client = New-Object NogbaScenarioInput+RECT
    if (-not [NogbaScenarioInput]::GetClientRect($window, [ref]$client)) { throw 'Unable to read NO$GBA client bounds.' }
    $clientWidth = $client.Right - $client.Left
    $clientHeight = $client.Bottom - $client.Top
    if ($clientWidth -eq 256 -and $clientHeight -eq 384) { $cropX = 0; $cropY = 0 }
    elseif ($clientWidth -eq 260 -and $clientHeight -eq 392) { $cropX = 2; $cropY = 4 }
    else { throw "Unexpected NO`$GBA client size ${clientWidth}x${clientHeight}; use 1x stacked screens." }

    $origin = New-Object NogbaScenarioInput+POINT
    if (-not [NogbaScenarioInput]::ClientToScreen($window, [ref]$origin)) { throw 'Unable to locate NO$GBA client area.' }
    [NogbaScenarioInput]::SetCursorPos($origin.X + $cropX + $X, $origin.Y + $cropY + $Y) | Out-Null
    [NogbaScenarioInput]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero) # left down
    Start-Sleep -Milliseconds $HoldMilliseconds
    [NogbaScenarioInput]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero) # left up
}

$before = @(Get-Process -Name 'NO$GBA' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
if ($before.Count -gt 0) {
    throw 'Close NO$GBA before running a UX scenario; scenarios require a clean, owned emulator instance.'
}
$bootstrap = Join-Path $OutputDirectory '.bootstrap.png'
$result = [System.Collections.Generic.List[string]]::new()

try {
    # The capture utility owns the reliable launch/resume logic. Its bootstrap
    # image is deliberately removed: only named scenario captures are outputs.
    $launchArgs = @{ Launch = $true; OutputPath = $bootstrap }
    if ($Build) { $launchArgs.Build = $true }
    & $captureScript @launchArgs
    if (-not $?) { throw 'Unable to launch the canonical ROM in NO$GBA.' }
    Remove-Item -LiteralPath $bootstrap -ErrorAction SilentlyContinue
    # The Emulator window exists before the app's first complete frame. Give the
    # canonical ROM a stable frame before injecting the first UX action.
    Start-Sleep -Milliseconds 2500

    $index = 0
    foreach ($step in $scenario.steps) {
        $index++
        $action = [string]$step.action
        switch ($action) {
            'wait' {
                if ($null -eq $step.ms) { throw "Step $index (wait) requires ms." }
                Start-Sleep -Milliseconds ([int]$step.ms)
            }
            'touch' {
                if ($null -eq $step.x -or $null -eq $step.y) { throw "Step $index (touch) requires x and y." }
                $hold = if ($null -eq $step.hold_ms) { 120 } else { [int]$step.hold_ms }
                Invoke-NogbaTouch -X ([int]$step.x) -Y ([int]$step.y) -HoldMilliseconds $hold
            }
            'manual' {
                if ([string]::IsNullOrWhiteSpace($step.instructions)) { throw "Step $index (manual) requires instructions." }
                Write-Host "UX_SCENARIO_MANUAL step=$index instructions=$($step.instructions)"
                Read-Host 'Completa el estado en NO$GBA y pulsa INTRO aquí para continuar' | Out-Null
            }
            'capture' {
                if ([string]::IsNullOrWhiteSpace($step.name)) { throw "Step $index (capture) requires name." }
                $safeName = [IO.Path]::GetFileNameWithoutExtension([string]$step.name)
                if ([string]::IsNullOrWhiteSpace($safeName)) { throw "Step $index (capture) has an invalid name." }
                $output = Join-Path $OutputDirectory ($safeName + '.png')
                & $captureScript -OutputPath $output
                if (-not $?) { throw "Capture step $index failed." }
                $result.Add($output)
            }
            default { throw "Step $index has unsupported action '$action'. Supported actions: wait, touch, manual, capture." }
        }
    }
    Write-Output "NOGBA_UX_SCENARIO_RESULT=PASS name=$($scenario.name) captures=$($result.Count) output=$OutputDirectory"
    $result | ForEach-Object { Write-Output "NOGBA_UX_CAPTURE=$_" }
} finally {
    if (-not $KeepOpen) {
        Get-Process -Name 'NO$GBA' -ErrorAction SilentlyContinue |
            Where-Object { $_.Id -notin $before } |
            ForEach-Object { Stop-Process -Id $_.Id; $_.WaitForExit() }
    }
}
