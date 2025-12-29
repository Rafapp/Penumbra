#!/bin/bash
set -e

# Colors and formatting
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$SCRIPT_DIR/.."

# Defaults
HEADLESS=OFF
BUILD_DEBUG=false
BUILD_RELEASE=true
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
      BUILD_DEBUG=true
      BUILD_RELEASE=false
      shift
      ;;
    --release)
      BUILD_DEBUG=false
      BUILD_RELEASE=true
      shift
      ;;
    --both)
      BUILD_DEBUG=true
      BUILD_RELEASE=true
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
      echo "  --debug         Build debug version only"
      echo "  --release       Build release version only (default)"
      echo "  --both          Build both debug and release"
      echo "  --headless      Build headless version"
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

build_config() {
  local BUILD_TYPE=$1
  local BUILD_DIR="build-$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"
  
  echo -e "${BOLD}${CYAN}╔════════════════════════════════════════╗${NC}"
  echo -e "${BOLD}${CYAN}║  Building Penumbra (${BUILD_TYPE})${NC}"
  echo -e "${BOLD}${CYAN}╚════════════════════════════════════════╝${NC}"
  echo ""
  echo -e "${BOLD}Configuration:${NC}"
  echo -e "  Build Dir:   ${MAGENTA}$BUILD_DIR${NC}"
  echo -e "  Build Type:  ${MAGENTA}$BUILD_TYPE${NC}"
  echo -e "  Headless:    ${MAGENTA}$HEADLESS${NC}"
  echo -e "  Verify:      ${MAGENTA}$VERIFY${NC}"
  echo -e "  Optimize:    ${MAGENTA}$OPTIMIZED${NC}"
  echo ""
  
  cd "$PROJECT_DIR"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  
  echo -e "${BOLD}${BLUE}[1/2]${NC} ${BOLD}Configuring CMake...${NC}"
  cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DENABLE_OPTIMIZATIONS=$OPTIMIZED \
    -DPATHTRACER_HEADLESS=$HEADLESS \
    -DENABLE_VERIFY_TESTS=$VERIFY
  
  echo -e "${GREEN}✓ Configuration complete${NC}"
  echo ""
  echo -e "${BOLD}${BLUE}[2/2]${NC} ${BOLD}Building Penumbra with libraries:${NC}"
  echo -e "  ${CYAN}◆${NC} GLFW"
  echo -e "  ${CYAN}◆${NC} GLM"
  echo -e "  ${CYAN}◆${NC} Assimp"
  echo -e "  ${CYAN}◆${NC} TinyBVH"
  echo -e "  ${CYAN}◆${NC} OpenImageIO"
  echo -e "  ${CYAN}◆${NC} ImGui"
  echo ""
  
  cmake --build . --config $BUILD_TYPE
  BUILD_EXIT_CODE=$?
  
  if [ $BUILD_EXIT_CODE -ne 0 ]; then
    echo -e "${RED}✗ Build failed!${NC}"
    return 1
  fi
  
  echo ""
  echo -e "${GREEN}✓ $BUILD_TYPE build successful!${NC}"
  
  if [[ $VERIFY == "ON" ]]; then
    echo -e "${BOLD}${BLUE}Running library verification tests...${NC}"
    echo ""
    ctest --output-on-failure -VV
    echo ""
    echo -e "${GREEN}✓ All libraries verified!${NC}"
  fi
  
  echo ""
  return 0
}

# Main build logic
FAILED=false

if [ "$BUILD_DEBUG" = true ]; then
  if ! build_config "Debug"; then
    FAILED=true
  fi
  echo ""
fi

if [ "$BUILD_RELEASE" = true ]; then
  if ! build_config "Release"; then
    FAILED=true
  fi
  echo ""
fi

if [ "$FAILED" = true ]; then
  echo -e "${BOLD}${RED}Build failed!${NC}"
  exit 1
fi

echo -e "${BOLD}${CYAN}╔════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║        Build Complete! 🎉              ║${NC}"
echo -e "${BOLD}${CYAN}╚════════════════════════════════════════╝${NC}"
echo -e "${BOLD}${GREEN}Ready to render!${NC}"
echo ""
echo -e "${CYAN}Build directories:${NC}"
echo -e "  Debug:   ${MAGENTA}./build-debug${NC}"
echo -e "  Release: ${MAGENTA}./build-release${NC}"
echo ""
