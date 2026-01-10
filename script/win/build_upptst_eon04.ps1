# PowerShell script to build Eon04 test for Windows
param(
    [switch]$Release,
    [switch]$NoDebugRt,
    [switch]$Uppheap,
    [switch]$Clean
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$genericScript = Join-Path $scriptDir "build_upptst_eon_generic.ps1"

$target = "Eon04"
$flags = "AI,SCREEN,SDL2,HAL,AUDIO,VIDEO,FBO,OGL,FFMPEG,OPENCV,CAMERA,SYS_PORTAUDIO"

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