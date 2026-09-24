@echo off
setlocal EnableExtensions

call "%~dp0setup_gcc.bat"
if errorlevel 1 exit /b 1

pushd "%~dp0.."
set "REPO=%CD%"
popd

set "SRC=%REPO%\userspace\spoofer.cxx"
set "OUT=%REPO%\dist\spoofer.exe"

if not exist "%REPO%\dist" mkdir "%REPO%\dist"

echo ============================================
echo   spoofer.exe
echo ============================================

gcc -m64 -O2 -Wall -o "%OUT%" "%SRC%" -ladvapi32 -lole32
if errorlevel 1 (
    echo [-] spoofer.cxx
    exit /b 1
)

if not exist "%OUT%" (
    echo [-] missing %OUT%
    exit /b 1
)

for %%A in ("%OUT%") do echo [+] %%~nxA %%~zA bytes
exit /b 0
