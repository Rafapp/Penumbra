@echo off
setlocal enabledelayedexpansion

REM Colors using ANSI escape codes (requires Windows 10+)
for /F %%A in ('echo prompt $H ^| cmd') do set "BS=%%A"
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "CYAN=[96m"
set "MAGENTA=[95m"
set "BOLD=[1m"
set "NC=[0m"

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."

REM Default values
set "HEADLESS=OFF"
set "BUILD_TYPE=Release"
set "VERIFY=ON"
set "OPTIMIZED=ON"

REM Parse arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="--headless" (
    set "HEADLESS=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto parse_args
)
if /i "%~1"=="--verify" (
    set "VERIFY=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--no-verify" (
    set "VERIFY=OFF"
    shift
    goto parse_args
)
if /i "%~1"=="--no-optimize" (
    set "OPTIMIZED=OFF"
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    echo Usage: build.bat [options]
    echo.
    echo Options:
    echo   --headless      Build headless version
    echo   --debug         Build debug version (default: Release)
    echo   --verify        Enable library verification (default: ON)
    echo   --no-verify     Skip library verification tests
    echo   --no-optimize   Disable optimizations
    echo   --help          Show this message
    exit /b 0
)
echo Unknown option: %~1
echo Use --help for usage information
exit /b 1

:args_done
cls
echo %BOLD%%CYAN%╔════════════════════════════════════════╗%NC%
echo %BOLD%%CYAN%║         Building Penumbra Pathtracer   ║%NC%
echo %BOLD%%CYAN%╚════════════════════════════════════════╝%NC%
echo.
echo %BOLD%Configuration:%NC%
echo   Headless:    %MAGENTA%!HEADLESS!%NC%
echo   Build Type:  %MAGENTA%!BUILD_TYPE!%NC%
echo   Verify:      %MAGENTA%!VERIFY!%NC%
echo   Optimize:    %MAGENTA%!OPTIMIZED!%NC%
echo.

cd /d "!PROJECT_DIR!"
if not exist build mkdir build
cd /d "!PROJECT_DIR!\build"

echo %BOLD%%BLUE%[1/2]%NC% %BOLD%Configuring CMake and fetching libraries...%NC%
cmake .. ^
  -DCMAKE_BUILD_TYPE=!BUILD_TYPE! ^
  -DENABLE_OPTIMIZATIONS=!OPTIMIZED! ^
  -DPATHTRACER_HEADLESS=!HEADLESS!

if errorlevel 1 (
    echo %RED%✗ CMake configuration failed!%NC%
    exit /b 1
)

echo %GREEN%✓ Configuration complete%NC%
echo.
echo %BOLD%%BLUE%[2/2]%NC% %BOLD%Building Penumbra with libraries:%NC%
echo   %CYAN%◆%NC% GLFW
echo   %CYAN%◆%NC% GLM
echo   %CYAN%◆%NC% Assimp
echo   %CYAN%◆%NC% TinyBVH
echo   %CYAN%◆%NC% OpenImageIO
echo   %CYAN%◆%NC% ImGui
echo.

cmake --build . --config !BUILD_TYPE!

if errorlevel 1 (
    echo %RED%✗ Build failed!%NC%
    echo.
    echo %BOLD%%RED%Please fix the errors above and try again.%NC%
    exit /b 1
)

echo.
echo %GREEN%✓ Build successful!%NC%
echo.

if "!VERIFY!"=="ON" (
    echo %BOLD%%BLUE%Running library verification tests...%NC%
    echo.
    ctest --output-on-failure -VV
    echo.
    echo %GREEN%✓ All libraries verified!%NC%
)

echo.
echo %BOLD%%CYAN%╔═══════════════════════════════╗%NC%
echo %BOLD%%CYAN%║        Build Complete!        ║%NC%
echo %BOLD%%CYAN%╚═══════════════════════════════╝%NC%
echo %BOLD%%GREEN%Ready to render!%NC%
echo.

endlocal