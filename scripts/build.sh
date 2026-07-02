#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${NUKE_ROOT:-}" ]]; then
  echo "NUKE_ROOT is not set. Example: export NUKE_ROOT='/opt/Nuke16.0v1'" >&2
  exit 1
fi

if [[ -n "${NUKE_VERSION:-}" ]]; then
  nuke_version="${NUKE_VERSION}"
else
  nuke_version="$(basename "${NUKE_ROOT}")"
  export NUKE_VERSION="${nuke_version}"
fi
plugin_output="build/plugin/$(uname -s)/${nuke_version}"

cmake -S . -B build -DNUKE_ROOT="${NUKE_ROOT}" -DNUKE_VERSION="${NUKE_VERSION}"
cmake --build build --config Release --target package_plugin

echo "Build completed. Plugin output is under ${plugin_output}."
echo "Using NUKE_VERSION=${NUKE_VERSION}."
echo "Package folder is under build/package (platform-specific subfolder)."
