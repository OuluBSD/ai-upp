param(
    [string]$HostName = $(if ($env:VIDEOSERVER_HOST) { $env:VIDEOSERVER_HOST } else { "127.0.0.1" }),
    [int]$Port = $(if ($env:VIDEOSERVER_PORT) { [int]$env:VIDEOSERVER_PORT } else { 8082 }),
    [int]$Minutes = $(if ($env:VIDEORECORDER_MINUTES) { [int]$env:VIDEORECORDER_MINUTES } else { 25 }),
    [int]$Fps = $(if ($env:VIDEORECORDER_FPS) { [int]$env:VIDEORECORDER_FPS } else { 10 }),
    [string]$Out = $env:VIDEORECORDER_OUT,
    [string]$WorkDir = $env:VIDEORECORDER_WORK_DIR,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoDir = Split-Path -Parent $scriptDir
$recorder = Join-Path $scriptDir "VideoRecorder.exe"

if (-not $Out) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $Out = Join-Path $scriptDir "video_record_${Minutes}min_${timestamp}.mp4"
}

if (-not $WorkDir) {
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($Out)
    $WorkDir = Join-Path $scriptDir "${baseName}_frames"
}

if (-not (Test-Path $recorder)) {
    Write-Error "VideoRecorder executable not found: $recorder. Build it with: bin\build.exe -m MSVS22x64 .\uppsrc\VideoRecorder\VideoRecorder.upp"
}

$arguments = @(
    "--host", $HostName,
    "--port", "$Port",
    "--minutes", "$Minutes",
    "--fps", "$Fps",
    "--out", $Out,
    "--work-dir", $WorkDir
)

if ($ExtraArgs) {
    $arguments += $ExtraArgs
}

Set-Location $repoDir

Write-Host "record_25min command:"
$printedArguments = ($arguments | ForEach-Object {
    if ($_ -match '\s') { "`"$_`"" } else { $_ }
}) -join " "
Write-Host ("`"{0}`" {1}" -f $recorder, $printedArguments)

& $recorder @arguments
exit $LASTEXITCODE
