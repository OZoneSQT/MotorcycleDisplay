@echo off
setlocal

set "SCRIPT_DIR=%~dp0"

where pwsh >nul 2>&1
if errorlevel 1 goto no_pwsh

pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Setup.ps1" %*
set "EXIT_CODE=%ERRORLEVEL%"
endlocal & exit /b %EXIT_CODE%

:no_pwsh
echo [ERROR] PowerShell 7 (pwsh) is required to run Setup.ps1.
echo Attempting to install PowerShell 7 via winget...

where winget >nul 2>&1
if errorlevel 1 (
    echo [ERROR] winget not found. Install PowerShell 7 manually and ensure "pwsh" is on PATH, then re-run this script.
    set "EXIT_CODE=1"
    endlocal & exit /b %EXIT_CODE%
)

set /p "REPLY=Do you want to install PowerShell (latest) using winget? [Y/N]: "
if /i "%REPLY%"=="Y" (
    echo Running: winget install --id Microsoft.PowerShell --e
    winget install --id Microsoft.PowerShell --e
    set "EXIT_CODE=%ERRORLEVEL%"
    if %EXIT_CODE% equ 0 (
        echo PowerShell 7 installation succeeded. Please re-run this script.
    ) else (
        echo [ERROR] winget install failed with exit code %EXIT_CODE%.
    )
) else (
    echo Installation cancelled. Please install PowerShell 7 manually and ensure "pwsh" is on PATH, then re-run this script.
    set "EXIT_CODE=1"
)

endlocal & exit /b %EXIT_CODE%
