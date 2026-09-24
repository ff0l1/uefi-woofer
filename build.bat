@echo off
setlocal EnableExtensions

cd /d "%~dp0"
if errorlevel 1 (
    echo [-] cannot cd to repo root
    exit /b 1
)

echo ============================================
echo   HWID Spoofer - full build
echo ============================================
echo.

call "%~dp0tools\setup_gcc.bat"
if errorlevel 1 exit /b 1

echo.

call "%~dp0tools\build_uefi.bat"
if errorlevel 1 (
    echo.
    echo [-] UEFI build failed
    exit /b 1
)

echo.

call "%~dp0tools\build_userspace.bat"
if errorlevel 1 (
    echo.
    echo [-] userspace build failed
    exit /b 1
)

echo.
if not exist "%~dp0dist\BOOTX64.EFI" (
    echo [-] dist\BOOTX64.EFI missing
    exit /b 1
)
if not exist "%~dp0dist\spoofer.exe" (
    echo [-] dist\spoofer.exe missing
    exit /b 1
)

echo ============================================
echo   OK  dist\BOOTX64.EFI
echo       dist\spoofer.exe
echo ============================================
exit /b 0
