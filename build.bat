@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 exit /b %errorlevel%
cl.exe /EHsc /Zi /std:c++17 /Fe:gitcopy.exe gitcopy.cpp
