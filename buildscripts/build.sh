#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$SCRIPT_DIR/.."

HEADLESS=OFF
BUILD_TYPE=Release
VERIFY=ON
OPTIMIZED=ON

# Parse arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --headless)
      HEADLESS=ON
      shift
      ;;
    --debug)
      BUILD_TYPE=Debug
      shift
      ;;
    --verify)
      VERIFY=ON
      shift
      ;;
    --no-verify)
      VERIFY=OFF
      shift
      ;;
    --no-optimize)
      OPTIMIZED=OFF
      shift
      ;;
    --help)
      echo "Usage: ./build.sh [options]"
      echo ""
      echo "Options:"
      echo "  --headless      Build headless version"
      echo "  --debug         Build debug version (default: Release)"
      echo "  --verify        Enable library verification (default: ON)"
      echo "  --no-verify     Skip library verification tests"
      echo "  --no-optimize   Disable optimizations"
      echo "  --help          Show this message"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      echo "Use --help for usage information"
      exit 1
      ;;
  esac
done

echo "=========================================="
echo "Building Penumbra"
echo "=========================================="
echo "Configuration:"
echo "  Headless:    $HEADLESS"
echo "  Build Type:  $BUILD_TYPE"
echo "  Verify:      $VERIFY"
echo "  Optimize:    $OPTIMIZED"
echo "=========================================="
echo ""

cd "$PROJECT_DIR"
mkdir -p build
cd build

echo "[1/3] Configuring CMake..."
cmake .. \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DENABLE_OPTIMIZATIONS=$OPTIMIZED \
  -DPATHTRACER_HEADLESS=$HEADLESS \
  -DENABLE_VERIFY_TESTS=$VERIFY

echo ""
echo "[2/3] Fetching and building libraries..."
echo "  - RGFW"
echo "  - GLM"
echo "  - ASSIMP"
echo "  - TinyBVH"
echo "  - ImGUI"
echo ""

echo "[3/3] Building Penumbra..."
cmake --build . --config $BUILD_TYPE

echo ""
echo "✓ Build successful!"
echo ""

if [[ $VERIFY == "ON" ]]; then
  echo "=========================================="
  echo "Running library verification tests..."
  echo "=========================================="
  ctest --output-on-failure -VV
  echo ""
  echo "✓ All libraries verified!"
fi

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="