[CmdletBinding()]
param(
    [string]$ReferenceRom = (Join-Path $PSScriptRoot '..\recovered\ds-stylophone-from-ds.nds'),
    [string]$CandidateRom = (Join-Path $PSScriptRoot '..\ds-stylophone.nds'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\artifacts\rom-compare')
)

$ErrorActionPreference = 'Stop'
$emulator = 'C:\Users\malfonso\OneDrive - ARQUIMEA GROUP\codigo_proyectos\personal\dsbard\.tools\nogba-debug\NO$GBA.EXE'

if (-not (Test-Path -LiteralPath $emulator)) { throw "NO$GBA no encontrado: $emulator" }
if (-not ('RomCompareAutomation' -as [type])) {
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class RomCompareAutomation {
  public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lParam);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);
  [DllImport("user32.dll")] public static extern IntPtr GetMenu(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern IntPtr GetSubMenu(IntPtr hMenu, int pos);
  [DllImport("user32.dll")] public static extern uint GetMenuItemID(IntPtr hMenu, int pos);
  [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT point);
}
'@
}

function Get-ProcessWindows([int]$ProcessId) {
    $result = [System.Collections.Generic.List[object]]::new()
    $callback = [RomCompareAutomation+EnumWindowsProc]{ param($handle, $unused)
        [uint32]$windowPid = 0
        [RomCompareAutomation]::GetWindowThreadProcessId($handle, [ref]$windowPid) | Out-Null
        if ($windowPid -eq $script:currentPid -and [RomCompareAutomation]::IsWindowVisible($handle)) {
            $title = [Text.StringBuilder]::new(256)
            [RomCompareAutomation]::GetWindowText($handle, $title, $title.Capacity) | Out-Null
            $script:windowResults.Add([pscustomobject]@{ Handle = $handle; Title = $title.ToString() })
        }
        return $true
    }
    $script:currentPid = $ProcessId
    $script:windowResults = $result
    [RomCompareAutomation]::EnumWindows($callback, [IntPtr]::Zero) | Out-Null
    return $result
}

function Capture-Rom([string]$RomPath, [string]$Label) {
    $fullRom = (Resolve-Path -LiteralPath $RomPath).Path
    $process = Start-Process -FilePath $emulator -WorkingDirectory (Split-Path -Parent $emulator) -ArgumentList ('"' + $fullRom + '"') -PassThru
    try {
        Start-Sleep -Seconds 2
        $windows = Get-ProcessWindows $process.Id
        $debugger = $windows | Where-Object Title -like '*Debugger*' | Select-Object -First 1
        if ($debugger) {
            $runMenu = [RomCompareAutomation]::GetSubMenu([RomCompareAutomation]::GetMenu($debugger.Handle), 2)
            $runCommand = [RomCompareAutomation]::GetMenuItemID($runMenu, 0)
            [RomCompareAutomation]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$runCommand, [IntPtr]::Zero) | Out-Null
        }
        Start-Sleep -Seconds 2
        $windows = Get-ProcessWindows $process.Id
        $game = $windows | Where-Object Title -like '*Emulator*' | Select-Object -First 1
        if (-not $game) { throw "Emulator window not found for $Label" }
        [RomCompareAutomation+RECT]$client = New-Object RomCompareAutomation+RECT
        if (-not [RomCompareAutomation]::GetClientRect($game.Handle, [ref]$client)) { throw "Cannot read window size for $Label" }
        $clientWidth = $client.R - $client.L
        $clientHeight = $client.B - $client.T
        if ($clientWidth -eq 256 -and $clientHeight -eq 384) { $cropX = 0; $cropY = 0 }
        elseif ($clientWidth -eq 260 -and $clientHeight -eq 392) { $cropX = 2; $cropY = 4 }
        else { throw "Unexpected NO$GBA client size ${clientWidth}x${clientHeight} for $Label" }
        [RomCompareAutomation+POINT]$origin = New-Object RomCompareAutomation+POINT
        [RomCompareAutomation]::ClientToScreen($game.Handle, [ref]$origin) | Out-Null
        Add-Type -AssemblyName System.Drawing
        $bitmap = [Drawing.Bitmap]::new(256, 384)
        try {
            $graphics = [Drawing.Graphics]::FromImage($bitmap)
            try { $graphics.CopyFromScreen($origin.X + $cropX, $origin.Y + $cropY, 0, 0, $bitmap.Size) }
            finally { $graphics.Dispose() }
            [IO.Directory]::CreateDirectory($OutputDirectory) | Out-Null
            $output = Join-Path $OutputDirectory "$Label.png"
            $bitmap.Save($output, [Drawing.Imaging.ImageFormat]::Png)
        } finally { $bitmap.Dispose() }
        [pscustomobject]@{ Label = $Label; Path = $output; Width = 256; Height = 384; Alive = -not $process.HasExited }
    } finally {
        if (-not $process.HasExited) { Stop-Process -Id $process.Id -Force; $process.WaitForExit() }
    }
}

$reference = Capture-Rom $ReferenceRom 'reference'
$candidate = Capture-Rom $CandidateRom 'candidate'
$refImage = [Drawing.Bitmap]::new($reference.Path)
$candidateImage = [Drawing.Bitmap]::new($candidate.Path)
try {
    $sum = [double]0
    $different = 0
    for ($y = 0; $y -lt 384; ++$y) {
        for ($x = 0; $x -lt 256; ++$x) {
            $a = $refImage.GetPixel($x, $y)
            $b = $candidateImage.GetPixel($x, $y)
            $distance = [math]::Abs($a.R - $b.R) + [math]::Abs($a.G - $b.G) + [math]::Abs($a.B - $b.B)
            $sum += $distance / 765.0
            if ($distance -gt 30) { ++$different }
        }
    }
    $pixels = 256 * 384
    Write-Output "ROM_UI_REFERENCE=$($reference.Path)"
    Write-Output "ROM_UI_CANDIDATE=$($candidate.Path)"
    Write-Output "ROM_UI_DIMENSIONS=256x384"
    Write-Output "ROM_UI_MEAN_PIXEL_DISTANCE=$([math]::Round($sum / $pixels, 4))"
    Write-Output "ROM_UI_DIFFERENT_PIXELS=$different/$pixels"
} finally {
    $refImage.Dispose()
    $candidateImage.Dispose()
}
