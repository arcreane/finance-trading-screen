@echo off
setlocal

set "QT_PATH=C:\Qt\6.10.0\msvc2022_64"
set "PATH=%QT_PATH%\bin;%PATH%"

echo Searching for Visual Studio...
set "VS_PATH="
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if "%VS_PATH%"=="" (
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community" set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\2022\Community"
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional" set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise" set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
)

if "%VS_PATH%"=="" (
  echo Error: Visual Studio not found.
  exit /b 1
)

echo Found VS at: %VS_PATH%
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

echo Running qmake...
qmake finance-trading-screen.pro
if %errorlevel% neq 0 exit /b %errorlevel%

echo Running nmake...
nmake
if %errorlevel% neq 0 exit /b %errorlevel%

echo Build successful.
