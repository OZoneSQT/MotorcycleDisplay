param(
	[switch]$SkipVenv,
	[switch]$Configure,
	[ValidateSet('Debug', 'Release')]
	[string]$BuildType = 'Debug'
)

if ($PSVersionTable.PSVersion.Major -lt 7) {
	$pwsh = Get-Command pwsh -ErrorAction SilentlyContinue
	if ($pwsh) {
		Write-Host "[INFO] Relaunching Setup.ps1 with PowerShell 7 (pwsh)." -ForegroundColor Yellow
		$pwshArgs = @('-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $MyInvocation.MyCommand.Path)
		foreach ($entry in $PSBoundParameters.GetEnumerator()) {
			$key = $entry.Key
			$value = $entry.Value
			if ($value -is [System.Management.Automation.SwitchParameter]) {
				if ($value.IsPresent) {
					$pwshArgs += "-$key"
				}
			} else {
				$pwshArgs += "-$key"
				$pwshArgs += $value
			}
		}
		& $pwsh.Source @pwshArgs
		exit $LASTEXITCODE
	}
	Write-Error 'PowerShell 7 (pwsh) is required to run Setup.ps1. Install PowerShell 7 and retry.'
	exit 1
}

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

function Import-VsDevEnvironment {
	param([string]$VsInstallPath)

	$vsDevCmd = Join-Path $VsInstallPath 'Common7/Tools/VsDevCmd.bat'
	if (-not (Test-Path $vsDevCmd)) {
		Write-WarnMessage "VsDevCmd not found at $vsDevCmd"
		return
	}

	Write-Section 'Importing Visual Studio developer environment'
	$cmdCommand = "`"$vsDevCmd`" -arch=amd64 -host_arch=amd64 >nul && set"
	$envLines = & cmd.exe /c $cmdCommand
	foreach ($line in $envLines) {
		if (-not $line) {
			continue
		}
		$separatorIndex = $line.IndexOf('=')
		if ($separatorIndex -lt 1) {
			continue
		}
		$name = $line.Substring(0, $separatorIndex)
		$value = $line.Substring($separatorIndex + 1)
		[System.Environment]::SetEnvironmentVariable($name, $value, 'Process')
		Set-Item -Path Env:$name -Value $value | Out-Null
	}
}

function EnsurePathEntry {
	param([string]$Directory)

	if (-not $Directory -or -not (Test-Path $Directory)) {
		return
	}

	$current = [System.Environment]::GetEnvironmentVariable('PATH', 'Process')
	$separator = ';'
	$entries = $current -split [System.Text.RegularExpressions.Regex]::Escape($separator)
	if ($entries -notcontains $Directory) {
		[System.Environment]::SetEnvironmentVariable('PATH', "$Directory$separator$current", 'Process')
		Write-Info "Added '$Directory' to PATH for this session."
	}
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

$llvmBin = Join-Path ${env:ProgramFiles} 'LLVM\bin'
EnsurePathEntry -Directory $llvmBin

$vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (Test-Path $vswherePath) {
	$vsInstallPath = & $vswherePath -products * -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null | Select-Object -First 1
	if ($vsInstallPath) {
		Import-VsDevEnvironment -VsInstallPath $vsInstallPath
		$clPath = & $vswherePath -products * -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'VC\Tools\MSVC\**\bin\Hostx64\x64\cl.exe' 2>$null | Select-Object -First 1
		if ($clPath) {
			EnsurePathEntry -Directory (Split-Path $clPath)
		}
	}
}

$clCompiler = Get-Command cl -ErrorAction SilentlyContinue
$clangCompiler = Get-Command clang++ -ErrorAction SilentlyContinue

if (-not $clCompiler -and -not $clangCompiler) {
	Write-WarnMessage 'No C++ compiler detected on PATH. Install Visual Studio Build Tools (with the Desktop development with C++ workload) or LLVM clang, then rerun Setup.ps1.'
}

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
	$clCompiler = Get-Command cl -ErrorAction SilentlyContinue
	$clangCompiler = Get-Command clang++ -ErrorAction SilentlyContinue
	if (-not $clCompiler -and -not $clangCompiler) {
		throw 'No C++ compiler found. Install Visual Studio Build Tools (C++ workload) or LLVM clang and ensure it is on PATH, then rerun with -Configure.'
	}
	Write-Section "Configuring CMake project"
	$buildDir = Join-Path $repoRoot 'build'
	if (-not (Test-Path $buildDir)) {
		New-Item -ItemType Directory -Path $buildDir | Out-Null
	}
	$cmakeArgs = @('-S', '.', '-B', 'build', '-G', 'Ninja', "-DCMAKE_BUILD_TYPE=$BuildType")
	if (-not $clCompiler -and $clangCompiler) {
		$cmakeArgs += @('-DCMAKE_C_COMPILER=clang', '-DCMAKE_CXX_COMPILER=clang++')
	}
	Invoke-CommandChecked -FilePath 'cmake' -Arguments $cmakeArgs -FriendlyName 'cmake configure'
}

Write-Info 'Setup complete. To activate the virtual environment run: .\.venv\Scripts\Activate.ps1'
