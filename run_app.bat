@echo off
setlocal
set "QT_PATH=C:\Qt\6.10.0\msvc2022_64"
set "PATH=%QT_PATH%\bin;%PATH%"

if exist "release\finance-trading-screen.exe" (
    echo Starting application...
    start "" "release\finance-trading-screen.exe"
) else (
    echo Error: Executable not found in release folder.
)
