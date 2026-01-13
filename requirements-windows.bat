@echo off
setlocal EnableDelayedExpansion

REM This script installs the required dependencies for Windows using vcpkg.
REM It expects vcpkg to be located at %USERPROFILE%\vcpkg or available in the system PATH.

set "VCPKG_EXE="

REM 1. Check for vcpkg in the user's home directory (preferred location)
if exist "%USERPROFILE%\vcpkg\vcpkg.exe" (
    set "VCPKG_EXE=%USERPROFILE%\vcpkg\vcpkg.exe"
    echo Found vcpkg at: !VCPKG_EXE!
) else (
    REM 2. If not found, check if vcpkg is in the PATH
    echo vcpkg not found in %USERPROFILE%\vcpkg, checking PATH...
    for /f "delims=" %%i in ('where vcpkg.exe 2^>nul') do (
        if not defined VCPKG_EXE set "VCPKG_EXE=%%i"
    )
    if defined VCPKG_EXE (
        echo Found vcpkg in PATH at: !VCPKG_EXE!
    )
)

REM 3. If vcpkg is not found, display an error and exit
if not defined VCPKG_EXE (
    echo.
    echo ERROR: vcpkg.exe not found.
    echo Please install vcpkg from https://vcpkg.io/
    echo and ensure it is either in your PATH or at %%USERPROFILE%%\vcpkg.
    exit /b 1
)

REM 4. List of packages to install for x64-windows
set "PACKAGES=sdl2 sdl2-image sdl2-ttf sdl2-mixer ffmpeg portaudio glew"
set "TRIPLET=x64-windows"
set "VCPKG_ARGS="
for %%p in (%PACKAGES%) do (
    set "VCPKG_ARGS=!VCPKG_ARGS! %%p:%TRIPLET%"
)
set "VCPKG_ARGS=!VCPKG_ARGS! llvm[clang,clang-tools-extra,bolt,mlir,target-webassembly,utils,lldb,lld]:%TRIPLET%"


echo.
echo Installing packages for %TRIPLET%...
echo !VCPKG_ARGS!
echo.

REM 5. Run the install command
"!VCPKG_EXE!" install !VCPKG_ARGS!

if !errorlevel! neq 0 (
    echo.
    echo ERROR: vcpkg package installation failed.
    exit /b 1
)

echo.
echo All required Windows dependencies installed successfully.
echo.

endlocal
exit /b 0
