#!/bin/bash -xe

BUILD_DIR="${BUILD_DIR:?BUILD_DIR variable is not defined}"
BUILD_TARGET="${BUILD_TARGET:-test}"

JOBS_NUM="$(( $(nproc 2>/dev/null || sysctl -n hw.ncpu) + 1 ))"
MAKE_ARGS="${MAKE_ARGS:-"-j$JOBS_NUM"}"

if [[ -n "$GITHUB_WORKSPACE" ]]; then
  git config --global --add safe.directory $GITHUB_WORKSPACE
fi
if [[ -n "$CI" && -f /venv/bin/activate ]]; then
  source /venv/bin/activate
fi

cd "$(dirname "$0")/.."

CMAKE_ARGS=( "-B" "${BUILD_DIR}" "$@" )

# see clang-tidy build in ci.yml github workflow
if [ "${BUILD_TARGET}" == "test" ]; then
  CMAKE_ARGS+=( "-DBUILD_TESTS=ON" )
fi

cmake . "${CMAKE_ARGS[@]}"

if [ "${BUILD_TARGET}" == "test" ]; then
  # first build test executables
  cmake --build "${BUILD_DIR}" -- "${MAKE_ARGS}"
fi
cmake --build "${BUILD_DIR}" --target "${BUILD_TARGET}" -- "${MAKE_ARGS}"
