@echo off
setlocal

set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR%"=="" set SCRIPT_DIR=.

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Setup.ps1" %*

endlocal
