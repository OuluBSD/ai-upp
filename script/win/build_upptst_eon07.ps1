# PowerShell script to build Eon07 test for Windows
param(
    [switch]$Release,
    [switch]$NoDebugRt,
    [switch]$Uppheap,
    [switch]$Clean,
    [switch]$Clang
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$genericScript = Join-Path $scriptDir "build_upptst_eon_generic.ps1"

# PowerShell treats -Clang and --Clang similarly in many contexts, 
# but let's ensure it's picked up.
if ($PSBoundParameters.ContainsKey('Clang')) {
    $Clang = $PSBoundParameters['Clang']
}

$target = "Eon07"
# Note: X11 is excluded for Windows compatibility, using SDL2 instead
$flags = "AI,SCREEN,SDL2,HAL,AUDIO,VIDEO,MEDIA,VOLUMETRIC,FBO,OGL,FFMPEG,CAMERA,PORTAUDIO"

# Select build model
$buildModel = if ($Clang) { "C:\Users\$env:USERNAME\upp\MSYS_CLANGx64.bm" } else { "C:\Users\$env:USERNAME\upp\MSVS22x64.bm" }

Write-Host "Launcher script: Clang switch is $Clang"
Write-Host "Launcher script: Selected BuildModel = $buildModel"

# Build the command with proper parameter passing
$paramArgs = @{
    Target = $target
    BaseFlags = $flags
    BuildModel = $buildModel
}

if ($Release) { $paramArgs.Release = $true }
if ($NoDebugRt) { $paramArgs.NoDebugRt = $true }
if ($Uppheap) { $paramArgs.Uppheap = $true }
if ($Clean) { $paramArgs.Clean = $true }

& $genericScript @paramArgs
