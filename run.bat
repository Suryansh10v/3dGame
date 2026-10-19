@echo off
setlocal
if exist "ChronoDash.exe" (
    echo Launching ChronoDash.exe...
    start "" "ChronoDash.exe"
    exit /b 0
)
if exist "build\ChronoDash.exe" (
    echo Launching build\ChronoDash.exe...
    start "" "build\ChronoDash.exe"
    exit /b 0
)
echo Executable not found. Running build.bat first...
call build.bat
if exist "ChronoDash.exe" start "" "ChronoDash.exe"
endlocal
