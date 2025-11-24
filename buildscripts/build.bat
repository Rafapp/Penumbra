# TODO: UNTESTED
@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..

set HEADLESS=OFF
set BUILD_TYPE=Release
set VERIFY=ON
set OPTIMIZED=ON

:parse_args
if "%1"=="" goto build
if "%1"=="--headless" (
  set HEADLESS=ON
  shift
  goto parse_args
)
if "%1"=="--debug" (
  set BUILD_TYPE=Debug
  shift
  goto parse_args
)
if "%1"=="--verify" (
  set VERIFY=ON
  shift
  goto parse_args
)
if "%1"=="--no-verify" (
  set VERIFY=OFF
  shift
  goto parse_args
)
if "%1"=="--no-optimize" (
  set OPTIMIZED=OFF
  shift
  goto parse_args
)
if "%1"=="--help" (
  echo Usage: build.bat [options]
  echo.
  echo Options:
  echo   --headless      Build headless version
  echo   --debug         Build debug version ^(default: Release^)
  echo   --verify        Enable library verification ^(default: ON^)
  echo   --no-verify     Skip library verification tests
  echo   --no-optimize   Disable optimizations
  echo   --help          Show this message
  exit /b 0
)

echo Unknown option: %1
echo Use --help for usage information
exit /b 1

:build
echo ==========================================
echo Building Penumbra
echo ==========================================
echo Configuration:
echo   Headless:    %HEADLESS%
echo   Build Type:  %BUILD_TYPE%
echo   Verify:      %VERIFY%
echo   Optimize:    %OPTIMIZED%
echo ==========================================
echo.

cd /d %PROJECT_DIR%
if not exist build mkdir build
cd build

echo [1/3] Configuring CMake...
cmake .. ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DENABLE_OPTIMIZATIONS=%OPTIMIZED% ^
  -DPATHTRACER_HEADLESS=%HEADLESS% ^
  -DENABLE_VERIFY_TESTS=%VERIFY%

if errorlevel 1 (
  echo CMake configuration failed!
  exit /b 1
)

echo.
echo [2/3] Fetching and building libraries...
echo   - RGFW
echo   - GLM
echo.

echo [3/3] Building Penumbra...
cmake --build . --config %BUILD_TYPE%

if errorlevel 1 (
  echo Build failed!
  exit /b 1
)

echo.
echo ✓ Build successful!
echo.

if "%VERIFY%"=="ON" (
  echo ==========================================
  echo Running library verification tests...
  echo ==========================================
  ctest --output-on-failure -VV
  
  if errorlevel 1 (
    echo Verification failed!
    exit /b 1
  )
  
  echo.
  echo ✓ All libraries verified!
)

echo.
echo ==========================================
echo Build complete!
echo ==========================================