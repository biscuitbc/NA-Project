@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"

REM ====================================================================
REM  run_demo.bat - one-click classroom demo for Windows
REM
REM  Usage:
REM    * Double-click this file, OR
REM    * In a terminal:        run_demo.bat
REM    * A single case:        run_demo.bat demo\demo2_zero_pivot.txt
REM
REM  Needs a C++17 compiler on PATH: g++, clang++, or cl (MSVC).
REM  For cl, run from a "Developer Command Prompt for VS".
REM ====================================================================

set "SRC=doolittle_partial_pivot.cpp"
set "BIN=build\doolittle_demo.exe"

if not exist build mkdir build

REM --- find a compiler: prefer g++, then clang++, then cl ---
set "CXX="
for %%C in (g++ clang++ cl) do (
    if not defined CXX (
        where %%C >nul 2>nul && set "CXX=%%C"
    )
)

if not defined CXX (
    echo [ERROR] No C++ compiler found ^(looked for g++, clang++, cl^).
    echo.
    echo Please install one of these, then run this file again:
    echo    * MinGW-w64 / MSYS2          provides g++
    echo    * LLVM                       provides clang++
    echo    * Visual Studio Build Tools  provides cl ^(use the
    echo      "Developer Command Prompt for VS" so cl is on PATH^)
    echo.
    pause
    exit /b 1
)

echo ==^> Compiling %SRC% with %CXX% ...
if /i "%CXX%"=="cl" (
    cl /nologo /EHsc /std:c++17 /utf-8 /O2 /Fo:build\ /Fe:%BIN% "%SRC%"
) else (
    %CXX% -std=c++17 -O2 -Wall -Wextra -o "%BIN%" "%SRC%"
)
if errorlevel 1 (
    echo.
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)
echo ==^> Build OK
echo.

set "CASES=%*"
if "%CASES%"=="" set "CASES=demo\*.txt"

for %%F in (%CASES%) do (
    echo ################################################################
    echo # %%F
    echo ################################################################
    echo ----- input -----
    type "%%F"
    echo ----- program output -----
    "%BIN%" < "%%F"
    echo.
)

echo ==^> All demo cases finished.
pause
