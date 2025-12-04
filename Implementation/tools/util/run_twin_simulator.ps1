[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$NoBuild
)

$ErrorActionPreference = 'Stop'

# Work from the Implementation root to ensure paths resolve consistently.
$implementationRoot = Resolve-Path -Path (Join-Path -Path $PSScriptRoot -ChildPath '..\..')
Set-Location -Path $implementationRoot

Write-Host '=== Motorcycle Twin Simulator Launcher ===' -ForegroundColor Cyan

# Always import the compiler toolchain and supporting utilities.
$setupArgs = @{
    SkipVenv  = $true
    BuildType = $Configuration
}
& "$implementationRoot\Setup.ps1" @setupArgs

if (-not $NoBuild.IsPresent) {
    Write-Host '[INFO] Building motorcycle_twin_simulator target' -ForegroundColor Green

    $cmakeCache = Join-Path -Path $implementationRoot -ChildPath 'build\CMakeCache.txt'
    if (-not (Test-Path -Path $cmakeCache)) {
        Write-Host '[INFO] Generating Ninja build files' -ForegroundColor Green
        & cmake -S $implementationRoot -B (Join-Path $implementationRoot 'build')
    }

    & cmake --build (Join-Path $implementationRoot 'build') --target motorcycle_twin_simulator --config $Configuration
}

$simulatorPath = Join-Path -Path $implementationRoot -ChildPath 'build\src\motorcycle_twin_simulator.exe'
if (-not (Test-Path -Path $simulatorPath)) {
    throw "Simulator executable not found at $simulatorPath. Run without -NoBuild to compile it."
}

Write-Host "[INFO] Launching $simulatorPath" -ForegroundColor Green
Start-Process -FilePath $simulatorPath -WorkingDirectory (Split-Path -Path $simulatorPath) | Out-Null
