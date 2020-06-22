@echo off

SET me=%~dp0

SET base=%me%..\code

IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

call %me%\..\bin\setup_cl_x64.bat


SET entry_point=%base%\platform_sdlgl.cpp
SET include_dir=
SET opts=-MT -WX -W4 -wd4201 -wd4996 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET linker_opts=-subsystem:console -libpath:%base%\thirdparty\sdl2\lib\x64
SET libs=SDL2main.lib SDL2.lib shell32.lib opengl32.lib
SET additional=-DDEBUG_OGL
SET common_flags=%opts% %additional% %entry_point% %include_dir% -link %libs% %linker_opts% 

copy %base%\thirdparty\sdl2\lib\x64\SDL2.dll %cd%

rmdir %cd%\shader
mkdir %cd%\shader
copy %base%\shader\* %cd%\shader

rmdir %cd%\assets
mkdir %cd%\assets
copy %base%\assets\* %cd%\assets

REM Make Debug

cl %common_flags%
