$ErrorActionPreference = 'Stop'

if (-not $env:NUKE_ROOT) {
    throw "NUKE_ROOT is not set. Example: `$env:NUKE_ROOT='C:/Program Files/Nuke16.0v1'"
}

$nukeVersion = if ($env:NUKE_VERSION) { $env:NUKE_VERSION } else { Split-Path -Path $env:NUKE_ROOT -Leaf }
$env:NUKE_VERSION = $nukeVersion
$pluginOutput = Join-Path -Path "build/plugin/Windows" -ChildPath $nukeVersion

cmake -S . -B build -DNUKE_ROOT="$env:NUKE_ROOT" -DNUKE_VERSION="$env:NUKE_VERSION"
cmake --build build --config Release --target package_plugin

Write-Host "Build completed. Plugin output is under $pluginOutput." -ForegroundColor Green
Write-Host "Using NUKE_VERSION=$env:NUKE_VERSION" -ForegroundColor Green
Write-Host "Package folder is under build/package (platform-specific subfolder)." -ForegroundColor Green
