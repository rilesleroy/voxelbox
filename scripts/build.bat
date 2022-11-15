@echo off
call scripts\shell.bat

set compile_flags= /nologo /Zi /FC /MT

rem LIBRARIES
set source= /I ..\src\
set glfw_include= /I ..\extlib\glfw\include\
set glad_headers= /I ..\extlib\glad\include\
set glad_src= /I ..\extlib\glad\src\
set nuklear_headers= /I ..\extlib\nuklear\
set stb_headers= /I ..\extlib\stb\

set lib_includes= %source% %glfw_include% %glad_headers% %glad_src% %nuklear_headers% %stb_headers%

echo "setting up links..."
set platform_links= /incremental:no /Debug:full gdi32.lib user32.lib kernel32.lib Shell32.lib opengl32.lib
set glfw_links= ..\extlib\glfw\lib-vc2019\glfw3_mt.lib
set links= %platform_links% %glfw_links%

if not exist build mkdir build
pushd build
echo "building..."
cl %compile_flags% %lib_includes% ..\src\glfw_platform.c /link %links%
echo "copying data..."
xcopy /y /E /I ..\data\ data\
popd