@echo off
setlocal
cd /d "%~dp0"
py -3 -c "import sys" >nul 2>nul
if not errorlevel 1 (
  py -3 tools\serve_playtest.py
) else (
  powershell.exe -NoProfile -File "%~dp0tools\serve_playtest.ps1"
)
if errorlevel 1 (
  echo Local server unavailable. Opening the extracted game directly.
  start "" "%~dp0index.html"
  pause
)
