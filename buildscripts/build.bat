@echo off
setlocal ENABLEDELAYEDEXPANSION

REM ============================================================
REM   ENABLE UTF-8 + ANSI COLORS ON WINDOWS TERMINAL / CMD
REM ============================================================
chcp 65001 >nul

REM Enable ANSI VT if not already enabled
for /f "tokens=2 delims=: " %%A in ('reg query HKCU\Console /v VirtualTerminalLevel 2^>nul') do set VTL=%%A
if not defined VTL (
    reg add HKCU\Console /f /v VirtualTerminalLevel /t REG_DWORD /d 1 >nul
)

REM ============================================================
REM   COLOR CONSTANTS (ESC =  character)
REM ============================================================
set "ESC="
set "RED=%ESC%[31m"
set "GREEN=%ESC%[32m"
set "YELLOW=%ESC%[33m"
set "BLUE=%ESC%[34m"
set "MAGENTA=%ESC%[35m"
set "CYAN=%ESC%[36m"
set "BOLD=%ESC%[1m"
set "NC=%ESC%[0m"

REM ============================================================
REM   SCRIPT LOCATION + DEFAULTS
REM ============================================================
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."

set "HEADLESS=OFF"
set "BUILD_TYPE=Release"
set "VERIFY=ON"
set "OPTIMIZED=ON"

REM ============================================================
REM   ARGUMENT PARSING
REM ============================================================
:parse_args
if "%~1"=="" goto args_done

if /i "%~1"=="--headless" (
    set "HEADLESS=ON"
    shift
    goto parse_args
)

if /i "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
	set "OPTIMIZED=OFF"
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
    echo   --debug         Build Debug instead of Release
    echo   --verify        Run verification tests
    echo   --no-verify     Skip verification tests
    echo   --no-optimize   Disable optimizations
    echo   --help          Show this help
    exit /b 0
)

echo Unknown option: %~1
exit /b 1

:args_done
cls

REM ============================================================
REM   BANNER
REM ============================================================
echo %CYAN%%BOLD%╔══════════════════════════════════════════════╗%NC%
echo %CYAN%%BOLD%║         Building Penumbra Pathtracer         ║%NC%
echo %CYAN%%BOLD%╚══════════════════════════════════════════════╝%NC%

echo %BOLD%Configuration:%NC%
echo   Headless:    %MAGENTA%!HEADLESS!%NC%
echo   Build Type:  %MAGENTA%!BUILD_TYPE!%NC%
echo   Verify:      %MAGENTA%!VERIFY!%NC%
echo   Optimize:    %MAGENTA%!OPTIMIZED!%NC%
echo.

REM ============================================================
REM   BUILD DIRECTORY
REM ============================================================
cd /d "%PROJECT_DIR%"
if not exist build mkdir build
cd build

REM ============================================================
REM   STEP 1: CMAKE CONFIG
REM ============================================================
echo %BLUE%%BOLD%[1/2]%NC% %BOLD%Configuring CMake...%NC%

cmake .. ^
  -DCMAKE_BUILD_TYPE=!BUILD_TYPE! ^
  -DENABLE_OPTIMIZATIONS=!OPTIMIZED! ^
  -DPATHTRACER_HEADLESS=!HEADLESS!

if errorlevel 1 (
    echo %RED%✗ CMake configuration FAILED%NC%
    exit /b 1
)

echo %GREEN%✓ Configuration complete%NC%
echo.

REM ============================================================
REM   STEP 2: BUILD
REM ============================================================
echo %BLUE%%BOLD%[2/2]%NC% %BOLD%Building Penumbra...%NC%
cmake --build . --config !BUILD_TYPE!

if errorlevel 1 (
    echo %RED%✗ Build FAILED%NC%
    exit /b 1
)

echo %GREEN%✓ Build successful!%NC%
echo.

REM ============================================================
REM   OPTIONAL: LIBRARY VERIFICATION
REM ============================================================
if /i "!VERIFY!"=="ON" (
    echo %BLUE%%BOLD%Running verification tests...%NC%
    echo.
    ctest --output-on-failure -VV
    echo.
    echo %GREEN%✓ All libraries verified!%NC%
)

echo.
echo %CYAN%%BOLD%╔═══════════════════════════════╗%NC%
echo %CYAN%%BOLD%║        Build Complete!         ║%NC%
echo %CYAN%%BOLD%╚═══════════════════════════════╝%NC%
echo %GREEN%%BOLD%Ready to render!%NC%
echo.

REM ============================================================
REM   RUN EXECUTABLE AFTER BUILD
REM ============================================================
echo %BLUE%%BOLD%Launching Penumbra...%NC%
echo.

set "RUN_DIR=%PROJECT_DIR%\build\%BUILD_TYPE%"
set "EXE_PATH=%RUN_DIR%\penumbra.exe"

if exist "!EXE_PATH!" (
    echo %GREEN%Running: !EXE_PATH!%NC%
    echo.
    start "" /D "!RUN_DIR!" "penumbra.exe"
) else (
    echo %RED%✗ Could not find penumbra.exe in %BUILD_TYPE% folder.%NC%
    echo Expected: !EXE_PATH!
)

endlocal
