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

HEADLESS=OFF
BUILD_TYPE=Release
VERIFY=ON
OPTIMIZED=ON

# Spinner function
spinner() {
  local pid=$1
  local delay=0.1
  local spinstr='⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏'
  
  while kill -0 $pid 2>/dev/null; do
    local temp=${spinstr#?}
    printf " ${CYAN}${spinstr:0:1}${NC}"
    spinstr=$temp${spinstr%"${temp}"}
    sleep $delay
    printf "\b\b\b"
  done
  printf "   \b\b\b"
}

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

echo -e "${BOLD}${CYAN}╔════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║         Building Penumbra Pathtracer   ║${NC}"
echo -e "${BOLD}${CYAN}╚════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BOLD}Configuration:${NC}"
echo -e "  Headless:    ${MAGENTA}$HEADLESS${NC}"
echo -e "  Build Type:  ${MAGENTA}$BUILD_TYPE${NC}"
echo -e "  Verify:      ${MAGENTA}$VERIFY${NC}"
echo -e "  Optimize:    ${MAGENTA}$OPTIMIZED${NC}"
echo ""

cd "$PROJECT_DIR"
mkdir -p build
cd build

echo -e "${BOLD}${BLUE}[1/2]${NC} ${BOLD}Configuring CMake and fetching libraries...${NC}"
cmake .. \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DENABLE_OPTIMIZATIONS=$OPTIMIZED \
  -DPATHTRACER_HEADLESS=$HEADLESS \

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
  echo ""
  echo -e "${BOLD}${RED}Please fix the errors above and try again.${NC}"
  exit 1
fi

echo ""
echo -e "${GREEN}✓ Build successful!${NC}"
echo ""

if [[ $VERIFY == "ON" ]]; then
  echo -e "${BOLD}${BLUE}Running library verification tests...${NC}"
  echo ""
  ctest --output-on-failure -VV
  echo ""
  echo -e "${GREEN}✓ All libraries verified!${NC}"
fi

echo ""
echo -e "${BOLD}${CYAN}╔════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║        Build Complete! 🎉              ║${NC}"
echo -e "${BOLD}${CYAN}╚════════════════════════════════════════╝${NC}"
echo -e "${BOLD}${GREEN}Ready to render some beautiful paths!${NC}"
echo ""
