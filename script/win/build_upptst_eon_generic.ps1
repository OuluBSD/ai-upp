# PowerShell script to build EON tests for Windows
param(
    [Parameter(Mandatory=$true)][string]$Target,
    [Parameter(Mandatory=$false)][string]$BaseFlags = "",
    [switch]$Release,
    [switch]$NoDebugRt,
    [switch]$Uppheap,
    [switch]$Clean
)

function Strip-Flag {
    param(
        [string]$Flags,
        [string]$FlagToRemove
    )

    $flagArray = $Flags -split ','
    $resultArray = $flagArray | Where-Object { $_ -ne $FlagToRemove -and $_ -ne "" }
    return ($resultArray -join ',')
}

function Append-Flag {
    param(
        [string]$Flags,
        [string]$FlagToAdd
    )

    $current = Strip-Flag -Flags $Flags -FlagToRemove $FlagToAdd
    if ([string]::IsNullOrEmpty($current)) {
        return $FlagToAdd
    } else {
        return "$current,$FlagToAdd"
    }
}

# Set build options
$enableDebugRt = if ($NoDebugRt -or $Release) { 0 } else { 1 }
$enableDebugFull = if ($Release) { 0 } else { 1 }
$useMalloc = if ($Uppheap) { 0 } else { 1 }
$cleanBuild = if ($Clean) { 1 } else { 0 }

# Build flags
$buildFlags = if ($Release) { "-bsH1" } else { "-bsdH1" }
if ($cleanBuild -eq 1) {
    $buildFlags += "a"
}

# Process flags
$finalFlags = Strip-Flag -Flags $BaseFlags -FlagToRemove "USEMALLOC"
$finalFlags = Strip-Flag -Flags $finalFlags -FlagToRemove "DEBUG_RT"
$finalFlags = Strip-Flag -Flags $finalFlags -FlagToRemove "DEBUG_FULL"

if ($useMalloc -eq 1) {
    $finalFlags = Append-Flag -Flags $finalFlags -FlagToAdd "USEMALLOC"
}

if ($enableDebugFull -eq 1) {
    $finalFlags = Append-Flag -Flags $finalFlags -FlagToAdd "DEBUG_FULL"
}

if ($enableDebugRt -eq 1) {
    $finalFlags = Append-Flag -Flags $finalFlags -FlagToAdd "DEBUG_RT"
}

# Ensure bin directory exists
if (!(Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" -Force
}

# Build the executable
if ($finalFlags) {
    $buildModel = "C:\Users\$env:USERNAME\upp\MSVS22x64.bm"
    $command = "umk ./upptst,./uppsrc `"$Target`" `"$buildModel`" `"$buildFlags`" `"+$finalFlags`" `"bin/$Target.exe`""
} else {
    $buildModel = "C:\Users\$env:USERNAME\upp\MSVS22x64.bm"
    $command = "umk ./upptst,./uppsrc `"$Target`" `"$buildModel`" `"$buildFlags`" `"bin/$Target.exe`""
}

Write-Host "Executing: $command"
Invoke-Expression $command

Write-Host "Executable compiled: bin/$Target.exe"
