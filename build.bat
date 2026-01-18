@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 exit /b %errorlevel%
setlocal enabledelayedexpansion
set "config=debug"

REM Check for -release flag
if "%1"=="-release" set "config=release"

set "temp_dir=temp"
set "bin_dir=bin\!config!"

if not exist "!temp_dir!" mkdir "!temp_dir!"
if not exist "!bin_dir!" mkdir "!bin_dir!"

cl.exe /EHsc /Zi /std:c++17 /Fo"!temp_dir!\\" /Fd"!temp_dir!\gitcopy.pdb" /Fe"!bin_dir!\gitcopy.exe" gitcopy.cpp
