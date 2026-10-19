@echo off
rem Build cores4.asi (x86 DLL). Usage: build.cmd "H:\SteamLibrary\steamapps\common\The Saboteur"
rem Needs Visual Studio Build Tools (C++); adjust the vcvars32 path below to your VS version.
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" >nul
cd /d "%~dp0"
cl /nologo /O1 /W3 /LD /Fe:cores4.asi cores4.c kernel32.lib user32.lib /link /DLL
if errorlevel 1 exit /b 1
if not "%~1"=="" copy /y cores4.asi "%~1\scripts\cores4.asi"
