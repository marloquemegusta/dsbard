[CmdletBinding()]
param([Parameter(Mandatory)][string]$ScenarioPath, [switch]$Build, [string]$OutputDirectory = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$scenario = Get-Content -Raw -LiteralPath (Resolve-Path $ScenarioPath) | ConvertFrom-Json
if (-not $scenario.name -or -not $scenario.steps) { throw 'Scenario needs name and steps.' }
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $root "artifacts\melonds\$($scenario.name)" }
[IO.Directory]::CreateDirectory($OutputDirectory) | Out-Null
if (Get-Process melonDS -ErrorAction SilentlyContinue) { throw 'Close melonDS before running a UX scenario.' }
if (-not ('MelonScenario' -as [type])) { Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class MelonScenario {
 [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
 [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
 [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h,out RECT r);
 [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h,ref POINT p);
 [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
 [DllImport("user32.dll")] public static extern bool SetCursorPos(int x,int y);
 [DllImport("user32.dll")] public static extern void mouse_event(uint f,uint x,uint y,uint d,UIntPtr e);
 [DllImport("user32.dll")] public static extern void keybd_event(byte k,byte s,uint f,UIntPtr e);
}
'@ }
$exe = Join-Path $root '.tools\melonds\melonDS.exe'; $toml = Join-Path $root '.tools\melonds\melonDS.toml'; $rom = Join-Path $root 'ds-stylophone.nds'
$originalToml = Get-Content -Raw -LiteralPath $toml; $process = $null
function Set-Binding($text, $name, $value) { [regex]::Replace($text, "(?m)^$([regex]::Escape($name)) = -?\d+$", "$name = $value", 1) }
function Get-Geometry {
 [MelonScenario+RECT]$r=New-Object MelonScenario+RECT;[MelonScenario]::GetClientRect($script:process.MainWindowHandle,[ref]$r)|Out-Null;$w=$r.R-$r.L;$h=$r.B-$r.T;if($w -le 0 -or $h -le 0){throw 'melonDS client unavailable'};[MelonScenario+POINT]$o=New-Object MelonScenario+POINT;[MelonScenario]::ClientToScreen($script:process.MainWindowHandle,[ref]$o)|Out-Null;[pscustomobject]@{W=$w;H=$h;O=$o}
}
function Capture([string]$name) {
 $script:process | ForEach-Object { [MelonScenario]::SetForegroundWindow($_.MainWindowHandle) | Out-Null; Start-Sleep -Milliseconds 100 }
 $g=Get-Geometry;Add-Type -AssemblyName System.Drawing;$b=[Drawing.Bitmap]::new($g.W,$g.H);$d=[Drawing.Graphics]::FromImage($b);$d.CopyFromScreen($g.O.X,$g.O.Y,0,0,$b.Size);$d.Dispose();$path=Join-Path $script:OutputDirectory ($name+'.png');$b.Save($path,[Drawing.Imaging.ImageFormat]::Png);$b.Dispose();$hash=(Get-FileHash $path -Algorithm SHA256).Hash.ToLowerInvariant();Write-Output "MELONDS_UX_CAPTURE=$path size=$($g.W)x$($g.H) sha256=$hash"
}
try {
 if($Build){ & (Join-Path $root 'build.ps1'); if($LASTEXITCODE -ne 0){throw 'Build failed'} }
 $edited=$originalToml; foreach($binding in $scenario.bindings.psobject.Properties){$edited=Set-Binding $edited $binding.Name ([int]$binding.Value)};[IO.File]::WriteAllText($toml,$edited)
 $script:process=Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe) -ArgumentList ('"'+$rom+'"') -PassThru;Start-Sleep -Seconds 3
 foreach($step in $scenario.steps){
  switch([string]$step.action){
   'wait'{Start-Sleep -Milliseconds ([int]$step.ms)}
   'button'{ $key=[int]$scenario.bindings.($step.name);$hold=250;if($step.hold_ms){$hold=[int]$step.hold_ms};[MelonScenario]::SetForegroundWindow($process.MainWindowHandle)|Out-Null;[MelonScenario]::keybd_event($key,0,0,[UIntPtr]::Zero);Start-Sleep -Milliseconds $hold;[MelonScenario]::keybd_event($key,0,2,[UIntPtr]::Zero);Start-Sleep -Milliseconds 250 }
   'touch'{ $g=Get-Geometry;$scale=($g.W-2)/256.0;$top=$g.H-[int][math]::Round(384*$scale)-4;$hold=120;if($step.hold_ms){$hold=[int]$step.hold_ms};[MelonScenario]::SetForegroundWindow($process.MainWindowHandle)|Out-Null;[MelonScenario]::SetCursorPos($g.O.X+1+[int][math]::Round(([int]$step.x)*$scale),$g.O.Y+$top+[int][math]::Round(([int]$step.y)*$scale))|Out-Null;[MelonScenario]::mouse_event(2,0,0,0,[UIntPtr]::Zero);Start-Sleep -Milliseconds $hold;[MelonScenario]::mouse_event(4,0,0,0,[UIntPtr]::Zero);Start-Sleep -Milliseconds 200 }
   'drag'{ $g=Get-Geometry;$scale=($g.W-2)/256.0;$top=$g.H-[int][math]::Round(384*$scale)-4;$hold=120;if($step.hold_ms){$hold=[int]$step.hold_ms};$x0=$g.O.X+1+[int][math]::Round(([int]$step.x0)*$scale);$y0=$g.O.Y+$top+[int][math]::Round(([int]$step.y0)*$scale);$x1=$g.O.X+1+[int][math]::Round(([int]$step.x1)*$scale);$y1=$g.O.Y+$top+[int][math]::Round(([int]$step.y1)*$scale);[MelonScenario]::SetForegroundWindow($process.MainWindowHandle)|Out-Null;[MelonScenario]::SetCursorPos($x0,$y0)|Out-Null;[MelonScenario]::mouse_event(2,0,0,0,[UIntPtr]::Zero);Start-Sleep -Milliseconds $hold;[MelonScenario]::SetCursorPos($x1,$y1)|Out-Null;Start-Sleep -Milliseconds $hold;[MelonScenario]::mouse_event(4,0,0,0,[UIntPtr]::Zero);Start-Sleep -Milliseconds 200 }
   'capture'{Capture $step.name}
   default{throw "Unsupported action $($step.action)"}
  }
 }
 Write-Output "MELONDS_UX_SCENARIO_RESULT=PASS name=$($scenario.name) output=$OutputDirectory"
} finally { if($process -and -not $process.HasExited){Stop-Process -Id $process.Id;$process.WaitForExit()};[IO.File]::WriteAllText($toml,$originalToml) }
