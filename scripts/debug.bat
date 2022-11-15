@echo off
call scripts\shell.bat
set debugger= ..\debugger\remedybg.exe


rem pick your poison
rem %debugger% debug_ses.rdbg
rem devenv.exe Win32_Potassium.exe
rem devenv.exe Win32_Potassium.exe
pushd build
%debugger% glfw_platform.exe
popd