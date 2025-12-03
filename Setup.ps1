#requires -Version 7.0
param(
	[switch]$SkipVenv,
	[switch]$Configure,
	[ValidateSet('Debug', 'Release')]
	[string]$BuildType = 'Debug'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

function Write-Section {
	param([string]$Message)
	Write-Host "`n=== $Message ===" -ForegroundColor Cyan
}

function Write-Info {
	param([string]$Message)
	Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Write-WarnMessage {
	param([string]$Message)
	Write-Warning $Message
}

function EnsureTool {
	param(
		[Parameter(Mandatory = $true)][string]$CommandName,
		[Parameter(Mandatory = $true)][string]$WingetId,
		[Parameter(Mandatory = $true)][string]$FriendlyName
	)

	if (Get-Command $CommandName -ErrorAction SilentlyContinue) {
		Write-Info "$FriendlyName is already available."
		return
	}

	$winget = Get-Command winget -ErrorAction SilentlyContinue
	if (-not $winget) {
		Write-WarnMessage "winget is not available. Please install $FriendlyName manually and re-run the setup."
		return
	}

	Write-Section "Installing $FriendlyName"
	$wingetArgs = @(
		'install',
		'--exact',
		'--id', $WingetId,
		'--accept-package-agreements',
		'--accept-source-agreements',
		'--scope', 'user',
		'--silent'
	)

	$process = Start-Process -FilePath $winget.Source -ArgumentList $wingetArgs -Wait -PassThru
	if ($process.ExitCode -ne 0) {
		Write-WarnMessage "winget failed to install $FriendlyName (exit code $($process.ExitCode)). Install it manually and rerun the script."
		return
	}

	if (Get-Command $CommandName -ErrorAction SilentlyContinue) {
		Write-Info "$FriendlyName installation complete."
	} else {
		Write-WarnMessage "$FriendlyName still not found after installation."
	}
}

function Invoke-CommandChecked {
	param(
		[Parameter(Mandatory = $true)][string]$FilePath,
		[string[]]$Arguments = @(),
		[string]$FriendlyName = $FilePath
	)

	$displayArgs = if ($Arguments) { $Arguments -join ' ' } else { '' }
	Write-Info "Running: $FriendlyName $displayArgs"
	$process = Start-Process -FilePath $FilePath -ArgumentList $Arguments -Wait -NoNewWindow -PassThru
	if ($process.ExitCode -ne 0) {
		throw "$FriendlyName failed with exit code $($process.ExitCode)."
	}
}

Write-Section "Preparing repository"
$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $repoRoot
Write-Info "Repository root: $repoRoot"

Write-Section "Ensuring toolchain"
EnsureTool -CommandName 'python' -WingetId 'Python.Python.3.11' -FriendlyName 'Python 3.11'
EnsureTool -CommandName 'cmake' -WingetId 'Kitware.CMake' -FriendlyName 'CMake'
EnsureTool -CommandName 'ninja' -WingetId 'Ninja-build.Ninja' -FriendlyName 'Ninja'

$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
	throw 'Python is required but was not found. Install Python and rerun the setup.'
}

if (-not $SkipVenv) {
	Write-Section "Configuring Python virtual environment"
	$venvPath = Join-Path $repoRoot '.venv'
	if (-not (Test-Path $venvPath)) {
		Invoke-CommandChecked -FilePath $python.Source -Arguments @('-m', 'venv', $venvPath) -FriendlyName 'python -m venv'
	} else {
		Write-Info "Virtual environment already exists at $venvPath"
	}

	$venvPython = Join-Path $venvPath 'Scripts\python.exe'
	if (-not (Test-Path $venvPython)) {
		throw "Virtual environment at $venvPath is missing python executable. Delete the folder and rerun the script."
	}

	Invoke-CommandChecked -FilePath $venvPython -Arguments @('-m', 'pip', 'install', '--upgrade', 'pip') -FriendlyName 'pip upgrade'

	$requirements = Join-Path $repoRoot 'requirements.txt'
	if (Test-Path $requirements) {
		Invoke-CommandChecked -FilePath $venvPython -Arguments @('-m', 'pip', 'install', '-r', $requirements) -FriendlyName 'pip install -r requirements.txt'
	} else {
		Write-WarnMessage 'requirements.txt not found; skipping Python dependency installation.'
	}
} else {
	Write-WarnMessage 'Skipping virtual environment setup as requested.'
}

if ($Configure) {
	Write-Section "Configuring CMake project"
	$buildDir = Join-Path $repoRoot 'build'
	if (-not (Test-Path $buildDir)) {
		New-Item -ItemType Directory -Path $buildDir | Out-Null
	}
	Invoke-CommandChecked -FilePath 'cmake' -Arguments @('-S', '.', '-B', 'build', '-G', 'Ninja', "-DCMAKE_BUILD_TYPE=$BuildType") -FriendlyName 'cmake configure'
}

Write-Info 'Setup complete. To activate the virtual environment run: .\.venv\Scripts\Activate.ps1'
