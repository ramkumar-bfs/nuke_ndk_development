#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${NUKE_NDK_ROOT:-}" ]]; then
  echo "NUKE_NDK_ROOT is not set. Example: export NUKE_NDK_ROOT='/opt/NUKE_NDK'" >&2
  exit 1
fi

if [[ -z "${NUKE_VERSION:-}" ]]; then
  echo "NUKE_VERSION is not set. Example: export NUKE_VERSION='Nuke16.0v1'" >&2
  exit 1
fi
nuke_version="${NUKE_VERSION}"
plugin_output="build/plugin/$(uname -s)/${nuke_version}"

cmake -S . -B build -DNUKE_NDK_ROOT="${NUKE_NDK_ROOT}" -DNUKE_VERSION="${NUKE_VERSION}"
cmake --build build --config Release --target package_plugin

echo "Build completed. Plugin output is under ${plugin_output}."
echo "Using NUKE_VERSION=${NUKE_VERSION}."
echo "Using NUKE_NDK_ROOT=${NUKE_NDK_ROOT}."
echo "Package folder is under build/package (platform-specific subfolder)."
