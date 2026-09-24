@echo off
setlocal EnableExtensions

call "%~dp0setup_gcc.bat"
if errorlevel 1 exit /b 1

pushd "%~dp0.."
set "REPO=%CD%"
popd

set "UEFI_DIR=%REPO%\uefi"
set "INC=%UEFI_DIR%\include"
set "OBJ_DIR=%REPO%\dist\obj"
set "OUT=%REPO%\dist\BOOTX64.EFI"

set "CFLAGS=-m64 -c -ffreestanding -fno-stack-protector -mno-red-zone -fshort-wchar -Wall -O2 -std=gnu++20"

echo ============================================
echo   BOOTX64.EFI
echo ============================================

if not exist "%REPO%\dist" mkdir "%REPO%\dist"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

echo [1/3] compile
gcc %CFLAGS% -I"%INC%" "%UEFI_DIR%\efi_lib.cxx" -o "%OBJ_DIR%\efi_lib.o"
if errorlevel 1 (
    echo [-] efi_lib.cxx
    exit /b 1
)

gcc %CFLAGS% -I"%INC%" "%UEFI_DIR%\smbios_patch.cxx" -o "%OBJ_DIR%\smbios_patch.o"
if errorlevel 1 (
    echo [-] smbios_patch.cxx
    exit /b 1
)

gcc %CFLAGS% -I"%INC%" "%UEFI_DIR%\main.cxx" -o "%OBJ_DIR%\main.o"
if errorlevel 1 (
    echo [-] main.cxx
    exit /b 1
)

echo [2/3] link
gcc -m64 -nostdlib -Wl,--subsystem,10 -e efi_main ^
    -o "%OUT%" ^
    "%OBJ_DIR%\main.o" ^
    "%OBJ_DIR%\efi_lib.o" ^
    "%OBJ_DIR%\smbios_patch.o"
if errorlevel 1 (
    gcc -m64 -nostdlib -Wl,--subsystem,10 -e efi_main ^
        -o "%OUT%" ^
        "%OBJ_DIR%\main.o" ^
        "%OBJ_DIR%\efi_lib.o" ^
        "%OBJ_DIR%\smbios_patch.o" -lgcc
    if errorlevel 1 (
        echo [-] link
        exit /b 1
    )
)

echo [3/3] verify
if not exist "%OUT%" (
    echo [-] missing %OUT%
    exit /b 1
)

for %%A in ("%OUT%") do echo [+] %%~nxA %%~zA bytes
exit /b 0
