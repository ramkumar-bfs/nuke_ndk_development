# Nuke NDK Custom Node Starter

This project is a starter template for building a custom Nuke node with the NDK (C++ SDK).

## What is included

- `src/PassThroughLite.cpp`: Minimal custom Iop node.
- `CMakeLists.txt`: Build configuration for a loadable Nuke plugin module.
- `scripts/build.ps1`: PowerShell helper script to configure and build.
- `scripts/build.sh`: Bash helper script to configure and build on Linux.

## Prerequisites

- NDK dependencies copied under a shared folder in this layout.
- Windows include path: `<NUKE_NDK_ROOT>/winx64/<NUKE_VERSION>/include`
- Windows lib path: `<NUKE_NDK_ROOT>/winx64/<NUKE_VERSION>/lib`
- Linux include path: `<NUKE_NDK_ROOT>/linux/<NUKE_VERSION>/include`
- Linux lib path: `<NUKE_NDK_ROOT>/linux/<NUKE_VERSION>/lib`
- CMake 3.20+
- A C++ compiler toolchain:
  - Windows: Visual Studio Build Tools
  - Linux: GCC or Clang

## Build on Windows

1. Set NDK root and version once per terminal session:

   ```powershell
   $env:NUKE_NDK_ROOT = "D:/deps/NUKE_NDK"
   $env:NUKE_VERSION = "Nuke16.0v1"
   ```

2. Configure and build:

   ```powershell
   ./scripts/build.ps1
   ```

3. Result plugin path:

   - `build/plugin/Windows/Nuke16.0v1/PassThroughLite.dll`
   - Packaged folder: `build/package/Windows-x86_64/` (name can vary by generator/arch)

## Build on Linux

1. Set NDK root and version once per terminal session:

   ```bash
   export NUKE_NDK_ROOT="/opt/NUKE_NDK"
   export NUKE_VERSION="Nuke16.0v1"
   ```

2. Configure and build:

   ```bash
   ./scripts/build.sh
   ```

3. Result plugin path:

   - `build/plugin/Linux/Nuke16.0v1/PassThroughLite.so`
   - Packaged folder: `build/package/Linux-x86_64/` (name can vary by distro/arch)

## Load in Nuke

Set `NUKE_PATH` to include the plugin output folder:

```powershell
$env:NUKE_PATH = "$PWD/build/plugin/Windows/Nuke16.0v1"
```

Then launch Nuke from that terminal. The node appears under:

- `Filter/PassThroughLite`

Linux example:

```bash
export NUKE_PATH="$PWD/build/plugin/Linux/Nuke16.0v1:${NUKE_PATH}"
```

To deploy to another machine, copy the platform-specific folder under `build/package/` and
add that copied folder to `NUKE_PATH` on the target machine.

## Extend this node

Edit `src/PassThroughLite.cpp` and implement image processing in `engine(...)`.
