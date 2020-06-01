@echo off


IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

CALL setup_cl_x64.bat

pushd ../code


SET includes=/I ryoji /I sdl2\include
SET libs=SDL2main.lib SDL2.lib
SET lib_paths=/LIBPATH:sdl2\lib\x64
SET opts=/Zi /W4 /wd4310 

SET flags=%includes% sdl2_vigil.cpp %opts%    /link /subsystem:windows %lib_paths% %libs%



REM Make Debug

@echo on
cl %flags%

