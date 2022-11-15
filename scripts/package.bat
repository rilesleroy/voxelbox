@echo off
call shell.bat

set source = /I ..\src\
set glfw_headers= /I ..\extlib\glfw\include\
set glad_headers= /I ..\extlib\glad\include\

set compile_flags= -nologo /Zi /FC /MT
set platform_links= -incremental:no /NODEFAULTLIB:MSVCRT gdi32.lib user32.lib kernel32.lib Shell32.lib opengl32.lib

set glfw_links= ..\extlib\glfw\lib-vc2019\glfw3dll.lib

if not exist build mkdir build
pushd build
cl %glad_headers% %glfw_headers% %compile_flags% ..\src\main.c /link %platform_links% %glfw_links%
DEL *.pdb
DEL *.obj
RD /S /Q .vs
if exist ..\the_package.zip DEL ..\the_package.zip
tar.exe -a -c -f ..\the_package.zip *
popd
