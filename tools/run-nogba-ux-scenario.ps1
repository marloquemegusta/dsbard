[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$ScenarioPath,
    [string]$OutputDirectory = '',
    [switch]$Build,
    [switch]$KeepOpen,
    [ValidateRange(1, 16)][int]$MonitorNumber = 2,
    [ValidateSet('strict', 'global', 'vjoy')][string]$InputMode = 'strict'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$captureScript = Join-Path $PSScriptRoot 'capture-nogba-screen.ps1'
. (Join-Path $PSScriptRoot 'nogba-debug-common.ps1')
$scenarioFile = (Resolve-Path -LiteralPath $ScenarioPath).Path
$scenario = Get-Content -LiteralPath $scenarioFile -Raw | ConvertFrom-Json

if ([string]::IsNullOrWhiteSpace($scenario.name)) { throw 'Scenario JSON requires a non-empty "name".' }
if ($null -eq $scenario.steps -or $scenario.steps.Count -eq 0) { throw 'Scenario JSON requires at least one step.' }
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) { $OutputDirectory = Join-Path $root ('artifacts\nogba\' + $scenario.name) }
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
[IO.Directory]::CreateDirectory($OutputDirectory) | Out-Null

$defaultKeys = @{
    A=0x5a; B=0x58; X=0x53; Y=0x41; L=0x51; R=0x57
    START=0x0d; SELECT=0x08; UP=0x26; DOWN=0x28; LEFT=0x25; RIGHT=0x27
}
$keyMap = $defaultKeys.Clone()
if ($scenario.keymap) {
    foreach ($property in $scenario.keymap.PSObject.Properties) { $keyMap[$property.Name.ToUpperInvariant()] = [int]$property.Value }
}

$before = @(Get-Process -Name 'NO$GBA' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
if ($before.Count -gt 0) { throw 'Close NO$GBA before running a UX scenario; the runner requires an owned instance.' }
$bootstrap = Join-Path $OutputDirectory '.bootstrap.png'
$manifest = [ordered]@{
    name = [string]$scenario.name
    monitor = $MonitorNumber
    input_mode = $InputMode
    captures = [System.Collections.Generic.List[object]]::new()
    tty = [ordered]@{ path = $null; markers = @(); missing_markers = @() }
    steps = [System.Collections.Generic.List[object]]::new()
    result = 'RUNNING'
}
$ownedProcess = $null
$failure = $null
$script:lastTtyText = $null

function Get-OwnedNogbaProcess {
    $processes = @(Get-Process -Name 'NO$GBA' -ErrorAction SilentlyContinue | Where-Object { $_.Id -notin $before })
    if ($processes.Count -eq 0) { throw 'NO$GBA process was not launched.' }
    return $processes | Select-Object -First 1
}

function Get-GameWindow([int]$ProcessId) {
    $window = Get-NogbaWindows -ProcessId $ProcessId | Where-Object Title -match 'Emulator' | Select-Object -First 1
    if (-not $window) { throw 'NO$GBA Emulator window is not available.' }
    return $window
}

function Get-DebuggerWindow([int]$ProcessId) {
    $window = Get-NogbaWindows -ProcessId $ProcessId | Where-Object Title -match 'Debugger' | Select-Object -First 1
    if (-not $window) { throw 'NO$GBA Debugger window is not available.' }
    return $window
}

function Open-NogbaTty([int]$ProcessId) {
    $debugger = Get-DebuggerWindow -ProcessId $ProcessId
    $command = Get-NogbaMenuCommand -Window $debugger.Handle -SubMenuIndex 4 -ItemIndex 25
    [NogbaDebugWin32]::SendMessage($debugger.Handle, 0x0111, [IntPtr]$command, [IntPtr]::Zero) | Out-Null
    Start-Sleep -Milliseconds 400
    return (Get-NogbaWindows -ProcessId $ProcessId | Where-Object Title -like '*TTY Debug*' | Select-Object -First 1)
}

function Export-Tty([string]$Reason) {
    $ttyWindow = Get-NogbaWindows -ProcessId $ownedProcess.Id | Where-Object Title -like '*TTY Debug*' | Select-Object -First 1
    if (-not $ttyWindow) { $ttyWindow = Open-NogbaTty -ProcessId $ownedProcess.Id }
    if (-not $ttyWindow) { throw 'TTY Debug Messages window is not available.' }
    $debugger = Get-DebuggerWindow -ProcessId $ownedProcess.Id
    try {
        $text = Export-NogbaTtyText -Debugger $debugger.Handle -TtyWindow $ttyWindow.Handle
    } catch {
        # A second Copy can occasionally miss while NO$GBA repaints the TTY
        # window. Reuse the last exported text so the actual marker check
        # reports missing evidence instead of hiding it as a clipboard error.
        if ($null -eq $script:lastTtyText) { throw }
        $text = $script:lastTtyText
    }
    $script:lastTtyText = $text
    $path = Join-Path $OutputDirectory 'tty.txt'
    [IO.File]::WriteAllText($path, $text, [Text.Encoding]::UTF8)
    $manifest.tty.path = $path
    $manifest.tty.markers = @($scenario.tty.markers)
    $manifest.tty.missing_markers = @($scenario.tty.markers | Where-Object { $text -notmatch [regex]::Escape([string]$_) })
    $manifest.steps.Add([pscustomobject]@{ action = 'tty'; reason = $Reason; path = $path })
    return $text
}

try {
    if ($InputMode -eq 'vjoy') {
        $vjoy = Initialize-NogbaVJoy -DeviceId 1
        $manifest.vjoy = [ordered]@{
            device_id = $vjoy.DeviceId
            buttons = $vjoy.Buttons
            continuous_pov = $vjoy.ContinuousPov
            touch_mode = 'global-restored'
        }
    }
    $launchArgs = @{ Launch = $true; OutputPath = $bootstrap; MonitorNumber = $MonitorNumber }
    if ($Build) { $launchArgs.Build = $true }
    & $captureScript @launchArgs
    if (-not $?) { throw 'Unable to launch canonical ROM in NO$GBA.' }
    Remove-Item -LiteralPath $bootstrap -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 1200
    $ownedProcess = Get-OwnedNogbaProcess
    $game = Get-GameWindow -ProcessId $ownedProcess.Id
    $debugger = Get-DebuggerWindow -ProcessId $ownedProcess.Id
    $monitor = Get-NogbaMonitor -Number $MonitorNumber
    $manifest.monitor_bounds = [ordered]@{ left=$monitor.Work.Left; top=$monitor.Work.Top; right=$monitor.Work.Right; bottom=$monitor.Work.Bottom }

    if ($scenario.tty) { Export-Tty -Reason 'initial' | Out-Null }

    $index = 0
    foreach ($step in $scenario.steps) {
        $index++
        $action = [string]$step.action
        switch ($action) {
            'wait' { Start-Sleep -Milliseconds ([int]$step.ms) }
            'button' {
                $name = ([string]$step.name).ToUpperInvariant()
                $hold = if ($null -eq $step.hold_ms) { 120 } else { [int]$step.hold_ms }
                $stepKeyMap = $keyMap.Clone()
                if ($null -ne $step.vk) { $stepKeyMap[$name] = [int]$step.vk }
                $input = if ($InputMode -eq 'vjoy') {
                    Invoke-NogbaVJoyButton -Name $name -HoldMilliseconds $hold
                } elseif ($InputMode -eq 'global') {
                    Invoke-NogbaButtonGlobal -Window $debugger.Handle -Name $name -HoldMilliseconds $hold -KeyMap $stepKeyMap
                } else {
                    Invoke-NogbaButtonStrict -Window $debugger.Handle -Name $name -HoldMilliseconds $hold -KeyMap $stepKeyMap
                }
                $manifest.steps.Add([pscustomobject]@{ action='button'; name=$name; mode=$input.Mode; down=$input.Down; up=$input.Up })
            }
            'touch' {
                $x = [int]$step.x; $y = [int]$step.y
                if ($x -lt 0 -or $x -gt 255 -or $y -lt 192 -or $y -gt 383) { throw "Touch coordinates must be x=0..255, y=192..383; got x=$x y=$y." }
                $hold = if ($null -eq $step.hold_ms) { 120 } else { [int]$step.hold_ms }
                $input = if ($InputMode -eq 'vjoy' -or $InputMode -eq 'global') {
                    Invoke-NogbaTouchGlobal -Window $game.Handle -X $x -Y $y -HoldMilliseconds $hold
                } else {
                    Invoke-NogbaTouchStrict -Window $game.Handle -X $x -Y $y -HoldMilliseconds $hold
                }
                $manifest.steps.Add([pscustomobject]@{ action='touch'; x=$x; y=$y; mode=$input.Mode; down=$input.Down; up=$input.Up })
            }
            'capture' {
                $safeName = [IO.Path]::GetFileNameWithoutExtension([string]$step.name)
                $output = Join-Path $OutputDirectory ($safeName + '.png')
                & $captureScript -OutputPath $output -MonitorNumber $MonitorNumber
                if (-not $?) { throw "Capture step $index failed." }
                $manifest.captures.Add([pscustomobject]@{ name=$safeName; path=$output; kind='ds-screen' })
            }
            'manual' {
                if ([string]::IsNullOrWhiteSpace($step.instructions)) { throw "Step $index (manual) requires instructions." }
                Write-Host "UX_SCENARIO_MANUAL step=$index instructions=$($step.instructions)"
                Read-Host 'Completa el estado en NO$GBA y pulsa INTRO aquí para continuar' | Out-Null
                $manifest.steps.Add([pscustomobject]@{ action='manual'; instructions=[string]$step.instructions })
            }
            'tty' { Export-Tty -Reason ([string]$step.name) | Out-Null }
            default { throw "Step $index has unsupported action '$action'." }
        }
    }
    if ($scenario.tty -and $manifest.tty.path -eq $null) { Export-Tty -Reason 'final' | Out-Null }
    if ($manifest.tty.missing_markers.Count -gt 0) { throw "TTY markers missing: $($manifest.tty.missing_markers -join ', ')" }
    $manifest.result = 'PASS'
    Write-Output "NOGBA_UX_SCENARIO_RESULT=PASS name=$($scenario.name) captures=$($manifest.captures.Count) output=$OutputDirectory"
} catch {
    $failure = $_
    $manifest.result = 'FAIL'
    $manifest.reason = $_.Exception.Message
    Write-Output "NOGBA_UX_SCENARIO_RESULT=FAIL name=$($scenario.name) reason=$($_.Exception.Message)"
} finally {
    if ($InputMode -eq 'vjoy') { Stop-NogbaVJoy }
    $manifestPath = Join-Path $OutputDirectory 'manifest.json'
    $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
    if (-not $KeepOpen) {
        Get-Process -Name 'NO$GBA' -ErrorAction SilentlyContinue |
            Where-Object { $_.Id -notin $before } |
            ForEach-Object { Stop-Process -Id $_.Id -Force; $_.WaitForExit() }
    }
}

if ($failure) { exit 1 }
