[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$NoBuild
)

$ErrorActionPreference = 'Stop'

# Work from the Implementation root so relative paths remain stable.
$implementationRoot = Resolve-Path -Path (Join-Path -Path $PSScriptRoot -ChildPath '..\..')
Set-Location -Path $implementationRoot

Write-Host '=== Motorcycle Dashboard Test Runner ===' -ForegroundColor Cyan

# Ensure compiler toolchain, Python tooling, and environment variables are imported.
$setupArgs = @{
    SkipVenv  = $true
    BuildType = $Configuration
}
& "$implementationRoot\Setup.ps1" @setupArgs

if (-not $NoBuild.IsPresent) {
    Write-Host '[INFO] Building motorcycle_tests target' -ForegroundColor Green

    $cmakeCache = Join-Path -Path $implementationRoot -ChildPath 'build\CMakeCache.txt'
    if (-not (Test-Path -Path $cmakeCache)) {
        Write-Host '[INFO] Generating Ninja build files' -ForegroundColor Green
        & cmake -S $implementationRoot -B (Join-Path $implementationRoot 'build')
    }

    & cmake --build (Join-Path $implementationRoot 'build') --target motorcycle_tests --config $Configuration
}

$buildDir = Join-Path -Path $implementationRoot -ChildPath 'build'
if (-not (Test-Path -Path $buildDir)) {
    throw "Build directory not found at $buildDir. Run without -NoBuild to generate it."
}

Write-Host '[INFO] Running ctest suite' -ForegroundColor Green
& ctest --test-dir $buildDir --output-on-failure --build-config $Configuration
