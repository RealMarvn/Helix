#!/usr/bin/env bash
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEBUG_BUILD_DIR="${PROJECT_ROOT}/build/debug"
RELEASE_BUILD_DIR="${PROJECT_ROOT}/build/release"

echo "[setup] Project root: $PROJECT_ROOT"

# ------------------------------------------------------------
# Ensure Meson + Ninja exist
# ------------------------------------------------------------
if ! command -v meson >/dev/null 2>&1; then
  echo "[ERROR] Meson not found. Install via:"
  echo "  pip install meson ninja"
  exit 1
fi

# ------------------------------------------------------------
# Detect available CPU cores for parallel builds
# ------------------------------------------------------------
if command -v sysctl >/dev/null 2>&1; then
  JOBS=$(sysctl -n hw.logicalcpu || echo 4)
elif command -v nproc >/dev/null 2>&1; then
  JOBS=$(nproc)
else
  JOBS=4
fi

echo "[setup] Parallel jobs: $JOBS"

# ------------------------------------------------------------
# Helper: configure build directory only if missing
# ------------------------------------------------------------
configure_build_dir() {
  local BUILD_DIR="$1"
  local BUILDTYPE="$2"

  if [ ! -f "$BUILD_DIR/meson-private/coredata.dat" ]; then
    echo "[setup] Configuring Meson build: $BUILD_DIR (type=$BUILDTYPE)"
    mkdir -p "$BUILD_DIR"
    meson setup "$BUILD_DIR" "$PROJECT_ROOT" --buildtype "$BUILDTYPE"
  else
    echo "[setup] Build dir exists: $BUILD_DIR"
  fi
}

# ------------------------------------------------------------
# Build Debug
# ------------------------------------------------------------
echo
echo "==============================="
echo "[setup] DEBUG BUILD"
echo "==============================="
configure_build_dir "$DEBUG_BUILD_DIR" "debug"
meson compile -C "$DEBUG_BUILD_DIR" -j "$JOBS"

# ------------------------------------------------------------
# Build Release
# ------------------------------------------------------------
echo
echo "==============================="
echo "[setup] RELEASE BUILD"
echo "==============================="
configure_build_dir "$RELEASE_BUILD_DIR" "release"
meson compile -C "$RELEASE_BUILD_DIR" -j "$JOBS"

# ------------------------------------------------------------
# Install Git hooks for clang-format + clang-tidy
# ------------------------------------------------------------
HOOKS_DIR="$PROJECT_ROOT/.git/hooks"
PRE_COMMIT="$HOOKS_DIR/pre-commit"

if [ -d "$PROJECT_ROOT/.git" ]; then
  echo
  echo "==============================="
  echo "[setup] Installing Git hooks"
  echo "==============================="

  mkdir -p "$HOOKS_DIR"

  cat > "$PRE_COMMIT" << 'EOF'
#!/usr/bin/env bash
set -e

BUILD_DIR="build/debug"

if [ ! -d "$BUILD_DIR" ]; then
  echo "[pre-commit] Debug build directory missing."
  echo "Run ./setup.sh first."
  exit 1
fi

echo "[pre-commit] Running clang-format..."
meson compile -C "$BUILD_DIR" format
git add -u

echo "[pre-commit] Running clang-tidy..."
meson compile -C "$BUILD_DIR" tidy || true

echo "[pre-commit] OK"
exit 0
EOF

  chmod +x "$PRE_COMMIT"
  echo "[setup] Git hook installed: $PRE_COMMIT"
else
  echo "[setup] No .git directory found – skipping hook install"
fi

# ------------------------------------------------------------
# Summary
# ------------------------------------------------------------
echo
echo "==============================="
echo "[setup] DONE"
echo "==============================="
echo "Debug binary:   $DEBUG_BUILD_DIR/helix"
echo "Release binary: $RELEASE_BUILD_DIR/helix"
echo
echo "Run debug build:"
echo "  meson compile -C build/debug helix && ./build/debug/helix"
echo
echo "Run tests:"
echo "  meson test -C build/release"
echo