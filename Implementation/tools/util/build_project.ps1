[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'

# Align the working directory with the Implementation root so relative paths match CMake/Setup.
$implementationRoot = Resolve-Path -Path (Join-Path -Path $PSScriptRoot -ChildPath '..\..')
Set-Location -Path $implementationRoot

Write-Host '=== Building Motorcycle Dashboard Project ===' -ForegroundColor Cyan

# Ensure the toolchain (compiler, CMake, Ninja) is on PATH and optionally refresh the build tree.
$setupArgs = @('-SkipVenv', '-BuildType', $Configuration)
& "$implementationRoot\Setup.ps1" @setupArgs

# Configure the CMake build directory once so subsequent builds are incremental.
$cmakeCache = Join-Path -Path $implementationRoot -ChildPath 'build\CMakeCache.txt'
if (-not (Test-Path -Path $cmakeCache)) {
    Write-Host '[INFO] Generating Ninja build files' -ForegroundColor Green
    & cmake -S $implementationRoot -B (Join-Path $implementationRoot 'build')
}

Write-Host "[INFO] Invoking CMake build for configuration '$Configuration'" -ForegroundColor Green

$buildArgs = @('--build', 'build', '--target', 'motorcycle_dashboard', 'motorcycle_twin_simulator', 'motorcycle_tests')
if ($Configuration) {
    $buildArgs += @('--config', $Configuration)
}

& cmake @buildArgs

Write-Host '[INFO] Build completed successfully.' -ForegroundColor Green
