$ErrorActionPreference = 'Stop'

if (-not $env:NUKE_NDK_ROOT) {
    throw "NUKE_NDK_ROOT is not set. Example: `$env:NUKE_NDK_ROOT='D:/deps/NUKE_NDK'"
}

if (-not $env:NUKE_VERSION) {
    throw "NUKE_VERSION is not set. Example: `$env:NUKE_VERSION='16.0v1'"
}

$nukeVersion = $env:NUKE_VERSION
$env:NUKE_VERSION = $nukeVersion
$pluginOutput = Join-Path -Path "build/plugin/Windows" -ChildPath $nukeVersion

cmake -S . -B build -DNUKE_NDK_ROOT="$env:NUKE_NDK_ROOT" -DNUKE_VERSION="$env:NUKE_VERSION"
cmake --build build --config Release --target package_plugin

Write-Host "Build completed. Plugin output is under $pluginOutput." -ForegroundColor Green
Write-Host "Using NUKE_VERSION=$env:NUKE_VERSION" -ForegroundColor Green
Write-Host "Using NUKE_NDK_ROOT=$env:NUKE_NDK_ROOT" -ForegroundColor Green
Write-Host "Package folder is under build/package (platform-specific subfolder)." -ForegroundColor Green
