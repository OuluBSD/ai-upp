# PowerShell script to build Eon00 test for Windows
param(
    [switch]$Release,
    [switch]$NoDebugRt,
    [switch]$Uppheap,
    [switch]$Clean
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$genericScript = Join-Path $scriptDir "build_upptst_eon_generic.ps1"

$target = "Eon00"
$flags = "AI,AUDIO,DEBUG_VFS"

# Build the command with proper parameter passing
$paramArgs = @{
    Target = $target
    BaseFlags = $flags
}

if ($Release) { $paramArgs.Release = $true }
if ($NoDebugRt) { $paramArgs.NoDebugRt = $true }
if ($Uppheap) { $paramArgs.Uppheap = $true }
if ($Clean) { $paramArgs.Clean = $true }

& $genericScript @paramArgs

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if (Test-Path "upptst/Eon00/*.eon") {
    Copy-Item -Path "upptst/Eon00/*.eon" -Destination "bin" -Force
}
