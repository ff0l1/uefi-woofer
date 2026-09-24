@echo off
setlocal EnableExtensions EnableDelayedExpansion
set "_BIN="

if defined MINGW_BIN (
    if exist "!MINGW_BIN!\gcc.exe" set "_BIN=!MINGW_BIN!"
)

if not defined _BIN (
    for /f "delims=" %%G in ('where gcc 2^>nul') do (
        if not defined _BIN set "_BIN=%%~dpG"
    )
)

if not defined _BIN (
    for %%D in (
        "C:\msys64\ucrt64\bin"
        "C:\msys64\mingw64\bin"
        "C:\MinGW\bin"
        "C:\mingw64\bin"
        "%ProgramFiles%\msys64\ucrt64\bin"
        "%ProgramFiles%\msys64\mingw64\bin"
        "%ProgramFiles%\mingw-w64\mingw64\bin"
        "%ProgramFiles(x86)\msys64\ucrt64\bin"
        "%ProgramFiles(x86)\msys64\mingw64\bin"
        "%LocalAppData%\Programs\msys64\ucrt64\bin"
        "%LocalAppData%\Programs\msys64\mingw64\bin"
        "%LocalAppData%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
    ) do (
        if exist "%%~D\gcc.exe" (
            set "_BIN=%%~D"
        )
    )
)

if not defined _BIN (
    echo [-] gcc not found.
    echo.
    echo     MSYS2: pacman -S mingw-w64-ucrt-x86_64-gcc
    echo     https://www.msys2.org/
    echo.
    echo     Or set MINGW_BIN to your MinGW bin directory.
    echo.
    endlocal
    exit /b 1
)

set "PATH=!_BIN!;!PATH!"

where gcc >nul 2>&1
if errorlevel 1 (
    echo [-] gcc not on PATH.
    endlocal
    exit /b 1
)

for /f "delims=" %%M in ('gcc -dumpmachine 2^>nul') do set "_M=%%M"
echo !_M! | findstr /i "x86_64 amd64" >nul
if errorlevel 1 (
    echo [-] need x86_64 gcc, got: !_M!
    endlocal
    exit /b 1
)

for /f "tokens=*" %%V in ('gcc -dumpversion 2^>nul') do set "_VER=%%V"
echo [+] gcc !_VER! ^(!_M!^)

for %%I in ("!_BIN!") do endlocal & set "PATH=%%~I;%PATH%"
exit /b 0
