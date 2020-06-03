@echo off

SET me=%~dp0
SET curdir=%cd%

SET builddir=%me%..\build
SET codedir=%me%..\code

IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

call %me%\setup_cl_x64.bat

SET common_flags=%codedir%\platform_sdl2_vigil.cpp -I %codedir%\ryoji -I %codedir%\sdl2\include SDL2main.lib SDL2.lib shell32.lib -link -subsystem:console -LIBPATH:%codedir%\sdl2\lib\x64 

copy %codedir%\sdl2\lib\x64\SDL2.dll %curdir%

REM Make Debug

cl %common_flags%
