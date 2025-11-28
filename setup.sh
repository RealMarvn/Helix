#!/usr/bin/env bash
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DEBUG_BUILD_DIR="${PROJECT_ROOT}/build/debug"
RELEASE_BUILD_DIR="${PROJECT_ROOT}/build/release"

echo "[setup] Project root: ${PROJECT_ROOT}"

# Detect CMake
if ! command -v cmake >/dev/null 2>&1; then
  echo "[setup] ERROR: cmake not found. Please install CMake (>= 3.27) and retry."
  exit 1
fi


if command -v sysctl >/dev/null 2>&1; then
  JOBS=$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
elif command -v nproc >/dev/null 2>&1; then
  JOBS=$(nproc)
else
  JOBS=4
fi

echo
echo "==============================="
echo "[setup] Configuring Debug build"
echo "==============================="
mkdir -p "${DEBUG_BUILD_DIR}"
cmake -S "${PROJECT_ROOT}" -B "${DEBUG_BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug
cmake --build "${DEBUG_BUILD_DIR}" -j"${JOBS}"

echo
echo "================================="
echo "[setup] (Optional) Release build"
echo "================================="
mkdir -p "${RELEASE_BUILD_DIR}"
cmake -S "${PROJECT_ROOT}" -B "${RELEASE_BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${RELEASE_BUILD_DIR}" -j"${JOBS}"

if [ -d "${PROJECT_ROOT}/.git" ]; then
  HOOKS_DIR="${PROJECT_ROOT}/.git/hooks"
  PRE_COMMIT_HOOK="${HOOKS_DIR}/pre-commit"

  echo
  echo "================================"
  echo "[setup] Installing pre-commit hook"
  echo "================================"

  mkdir -p "${HOOKS_DIR}"

  cat > "${PRE_COMMIT_HOOK}" << 'EOF'
  #!/usr/bin/env bash

  BUILD_DIR="build/debug"

  echo "[pre-commit] Ensuring build directory exists..."
  if [ ! -d "$BUILD_DIR" ]; then
    echo "[pre-commit] Build directory '$BUILD_DIR' not found."
    echo "Run first:"
    echo "  mkdir -p $BUILD_DIR"
    echo "  cd $BUILD_DIR && cmake -DCMAKE_BUILD_TYPE=Debug ../.."
    exit 1
  fi

  echo "[pre-commit] Running clang-format (auto-fix)..."
  cmake --build "$BUILD_DIR" --target format

  if [ $? -ne 0 ]; then
    echo "[pre-commit] clang-format failed."
    exit 1
  fi

  # Re-add formatted files automatically
  echo "[pre-commit] Re-adding formatted files..."
  git add -u

  echo "[pre-commit] Running clang-tidy (optional)..."
  cmake --build "$BUILD_DIR" --target tidy || \
  echo "[pre-commit] clang-tidy reported issues (not blocking commit)"

  echo "[pre-commit] OK (formatted automatically)"
  exit 0
EOF

  chmod +x "${PRE_COMMIT_HOOK}"
  echo "[setup] pre-commit hook installed at .git/hooks/pre-commit"
else
  echo
  echo "[setup] No .git directory found, skipping hook installation."
fi

echo
echo "[setup] Done."
echo "[setup] Debug binary:   ${DEBUG_BUILD_DIR}/helix"
echo "[setup] Release binary: ${RELEASE_BUILD_DIR}/helix"
echo
echo "You can now run, for example:"
echo "  cd ${DEBUG_BUILD_DIR}"
echo "  cmake --build . --target run"