@echo off

SET me=%~dp0

SET base=%me%..\code

IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

call %me%\..\bin\setup_cl_x64.bat


SET entry_point=%base%\platform_sdl2_vigil.cpp
SET include_dir=-I %base%\ryoji -I %base%\yuu
SET opts=-MT -WX -W4 -wd4201 -Zi -Oi -GR- -EHa -Gm-
SET linker_opts=-subsystem:console -libpath:%base%\yuu\sdl2\lib\x64
SET libs=SDL2main.lib SDL2.lib shell32.lib 

SET common_flags=%opts% %entry_point% %include_dir% -link %libs% %linker_opts% 

copy %base%\thirdparty\sdl2\lib\x64\SDL2.dll %cd%

REM Make Debug

cl %common_flags%


