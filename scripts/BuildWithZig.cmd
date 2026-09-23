@echo off
setlocal

for %%I in ("%~dp0..") do set "PROJECT_DIR=%%~fI"
if defined ZIG_EXE goto :have_zig

for /f "delims=" %%I in ('where zig.exe 2^>nul') do if not defined ZIG_EXE set "ZIG_EXE=%%I"

:have_zig
if not defined ZIG_EXE (
    echo zig.exe was not found.
    echo Set the ZIG_EXE environment variable to the full path of zig.exe.
    exit /b 1
)

if not exist "%PROJECT_DIR%\bin" mkdir "%PROJECT_DIR%\bin"
set "BUILD_DIR=%PROJECT_DIR%\build\zig"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo Building MinHook objects...
"%ZIG_EXE%" cc -target x86_64-windows-gnu -O2 ^
  -I"%PROJECT_DIR%\src\common" ^
  -I"%PROJECT_DIR%\third_party\minhook\include" ^
  -I"%PROJECT_DIR%\third_party\minhook\src" ^
  -c "%PROJECT_DIR%\third_party\minhook\src\buffer.c" ^
  -o "%BUILD_DIR%\buffer.o"
if errorlevel 1 exit /b 1

"%ZIG_EXE%" cc -target x86_64-windows-gnu -O2 ^
  -I"%PROJECT_DIR%\third_party\minhook\include" ^
  -I"%PROJECT_DIR%\third_party\minhook\src" ^
  -c "%PROJECT_DIR%\third_party\minhook\src\hook.c" ^
  -o "%BUILD_DIR%\hook.o"
if errorlevel 1 exit /b 1

"%ZIG_EXE%" cc -target x86_64-windows-gnu -O2 ^
  -I"%PROJECT_DIR%\third_party\minhook\include" ^
  -I"%PROJECT_DIR%\third_party\minhook\src" ^
  -c "%PROJECT_DIR%\third_party\minhook\src\trampoline.c" ^
  -o "%BUILD_DIR%\trampoline.o"
if errorlevel 1 exit /b 1

"%ZIG_EXE%" cc -target x86_64-windows-gnu -O2 ^
  -I"%PROJECT_DIR%\third_party\minhook\src" ^
  -c "%PROJECT_DIR%\third_party\minhook\src\hde\hde64.c" ^
  -o "%BUILD_DIR%\hde64.o"
if errorlevel 1 exit /b 1

echo Building ICloudTrayMenu.dll...
"%ZIG_EXE%" c++ -target x86_64-windows-gnu -std=c++20 -O2 -shared -Wno-nullability-completeness ^
  -I"%PROJECT_DIR%\src\common" ^
  -I"%PROJECT_DIR%\third_party\minhook\include" ^
  "%PROJECT_DIR%\src\tray_menu\dllmain.cpp" ^
  "%BUILD_DIR%\buffer.o" ^
  "%BUILD_DIR%\hook.o" ^
  "%BUILD_DIR%\trampoline.o" ^
  "%BUILD_DIR%\hde64.o" ^
  -luser32 -lkernel32 -lapi-ms-win-core-winrt-l1-1-0 -lapi-ms-win-core-winrt-string-l1-1-0 ^
  -o "%PROJECT_DIR%\bin\ICloudTrayMenu.dll"
if errorlevel 1 exit /b 1

echo Building ICloudTrayBridge.exe...
"%ZIG_EXE%" c++ -target x86_64-windows-gnu -std=c++20 -O2 -municode -Wno-nullability-completeness ^
  -I"%PROJECT_DIR%\src\common" ^
  "%PROJECT_DIR%\src\bridge\main.cpp" ^
  -lshell32 -lkernel32 -Wl,--subsystem,windows ^
  -o "%PROJECT_DIR%\bin\ICloudTrayBridge.exe"
if errorlevel 1 exit /b 1

if exist "%PROJECT_DIR%\bin\dllmain.lib" move /y "%PROJECT_DIR%\bin\dllmain.lib" "%BUILD_DIR%\ICloudTrayMenu.lib" >nul
if exist "%PROJECT_DIR%\bin\ICloudTrayMenu.pdb" move /y "%PROJECT_DIR%\bin\ICloudTrayMenu.pdb" "%BUILD_DIR%\ICloudTrayMenu.pdb" >nul
if exist "%PROJECT_DIR%\bin\ICloudTrayBridge.pdb" move /y "%PROJECT_DIR%\bin\ICloudTrayBridge.pdb" "%BUILD_DIR%\ICloudTrayBridge.pdb" >nul

echo Build completed.
exit /b 0
